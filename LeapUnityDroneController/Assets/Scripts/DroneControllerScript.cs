using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

using Leap;
using Leap.Unity;

public class DroneControllerScript : MonoBehaviour
{
    [Tooltip("The LeapProvider to use")]
    [SerializeField]
    public LeapProvider _leapProvider;
    public GameObject leftStick;
    public GameObject rightStick;
    public Slider aileronSlider;
    public Slider elevatorSlider;
    public Slider rudderSlider;
    public Slider throttleSlider;
    public Dropdown COMDropdown;
    public float refreshPeriodMS = 20.0f; //so every 20ms we send data via serial to the drone
    //public LeapServiceProvider leapServiceProvider;
    //public LeapHandController leapHandController;
    //public com port
    public SerialCOM _serialCOM;

    Rect textArea = new Rect(0, 0, Screen.width, Screen.height);
    private float _handConfidence = 0;
    private float _aileron = 0;
    private float _elevator = 0;
    private float _rudder = 0;
    private float _throttle = 0;
    private float _grip = 0;
    private float timer = 0; //this is the time since we last sent an update via serial to the drone (milliseconds)

    /// <summary>
    /// Called when the COM port is set so that it can be opened and communications can start
    /// </summary>
    /// <param name="comName"></param>
    public void setCOMPort(string comName)
    {
        if (_serialCOM.isOpen) _serialCOM.closePort(); //safety check on an existing open port
        bool success = _serialCOM.openPort(comName);
        if (!success)
        {
            Debug.LogError("COM Port " + comName + " failed to open");
        }
    }

    protected virtual void OnEnable()
    {
        if (_leapProvider == null)
        {
            _leapProvider = Hands.Provider;
        }

        _leapProvider.OnUpdateFrame -= OnUpdateFrame;
        _leapProvider.OnUpdateFrame += OnUpdateFrame;

        _leapProvider.OnFixedFrame -= OnFixedFrame;
        _leapProvider.OnFixedFrame += OnFixedFrame;

        populateCOMDropdown();
    }

    protected virtual void OnDisable()
    {
        _leapProvider.OnUpdateFrame -= OnUpdateFrame;
        _leapProvider.OnFixedFrame -= OnFixedFrame;
    }

    // Start is called before the first frame update
    void Start()
    {
        timer = 0;
        COMDropdown.onValueChanged.AddListener( delegate { COMDropdownChanged(COMDropdown); });
    }

    // Update is called once per frame
    void Update()
    {
        timer += Time.deltaTime*1000.0f;
        if (timer>=refreshPeriodMS)
        {
            if (_serialCOM.isOpen)
            {
                _serialCOM.sendROGUEData(0, _aileron); //Ch1 on Tx
                _serialCOM.sendROGUEData(1, _elevator); //Ch2
                _serialCOM.sendROGUEData(2, _throttle); //Ch3
                //_serialCOM.sendROGUEData(3, _rudder); //Ch4
                //channel 4 is the flight mode (Ch5 on TX)
                if (_handConfidence>0.5)
                    _serialCOM.sendROGUEData(5, 1.0f); //Ch6 on TX - this is the arm switch, so we need it armed
                else
                    _serialCOM.sendROGUEData(5, -1.0f); //disarm if no hand detected
                _serialCOM.flush(); //need to flush the stream so we don't get synchronisation issues with messages getting buffered
            }
            timer = 0;
        }
    }

    void OnGUI()
    {
        GUI.Label(textArea,
            "a: " + _aileron.ToString() + "\n"
            + "e: " + _elevator.ToString() + "\n"
            + "r: " + _rudder.ToString() + "\n"
            + "t: " + _throttle.ToString() + "\n"
            + "grip: " + _grip + "\n"
            + "confidence: " + _handConfidence
        );
    }

    #region COMDropdown functions
    /// <summary>
    /// UI Handler.
    /// Called on response to the COM Port Dropdown control value being changed. Listener is set up in Start() function.
    /// </summary>
    /// <param name="dropdown">The control where the change happened.</param>
    public void COMDropdownChanged(Dropdown dropdown)
    {
        string name = dropdown.options[dropdown.value].text;
        Debug.Log("COM Port change: " + name);
        setCOMPort(name); //sets and opens port
    }

    /// <summary>
    /// Fill the COM dropdown box with any COM ports we find that are active - one of these should be the Arduino
    /// </summary>
    protected void populateCOMDropdown()
    {
        COMDropdown.ClearOptions();
        string[] portNames = SerialCOM.listPorts();
        foreach (string name in portNames) COMDropdown.options.Add(new Dropdown.OptionData(name));
    }

    #endregion COMDropdown functions

    protected void UpdateSticksGraphics()
    {
        //this.leftStick.transform.eulerAngles = new Vector3(Mathf.Rad2Deg * _throttle, -Mathf.Rad2Deg * _rudder, 0);
        //this.rightStick.transform.eulerAngles = new Vector3(-Mathf.Rad2Deg * _elevator, -Mathf.Rad2Deg*_aileron, 0);
        //this.rightStickPivotAileron.transform.rotation = Quaternion.AngleAxis(-Mathf.Rad2Deg * _aileron, Vector3.up);
        //this.rightStickPivotElevator.transform.rotation = Quaternion.AngleAxis(-Mathf.Rad2Deg * _elevator, Vector3.right);
        //this.rightStickPivotElevator.transform.localRotation = Quaternion.AngleAxis(-Mathf.Rad2Deg * _elevator, Vector3.right);
        this.leftStick.transform.localPosition = new Vector3(_rudder * 100, _throttle * 100, 0);
        this.rightStick.transform.localPosition = new Vector3(_aileron * 100, _elevator * 100, 0);

        //now the sliders
        aileronSlider.value = _aileron;
        elevatorSlider.value = _elevator;
        rudderSlider.value = _rudder;
        throttleSlider.value = _throttle;
    }

    /** Updates the graphics HandRepresentations. */
    protected virtual void OnUpdateFrame(Frame frame)
    {
        _aileron = 0;
        _elevator = 0;
        _rudder = 0;
        _throttle = -1; //NOTE: -1 throttle is NO THROTTLE - this is a safety feature!
        if (frame != null)
        {
            Hand hand = null;
            _handConfidence = 0;
            foreach (Hand h in frame.Hands) //find hand with the best confidence - we don't care which one
            {
                if (h.Confidence > _handConfidence)
                {
                    hand = h;
                    _handConfidence = h.Confidence;
                }
            }

            if ((hand!=null)&&(_handConfidence>0.8))
            {
                //check hands number and confidence
                //UpdateHandRepresentations(graphicsHandReps, ModelType.Graphics, frame);
                _aileron = -hand.PalmNormal.Roll;
                if (_aileron < -1.0f) _aileron = -1.0f;
                else if (_aileron > 1.0f) _aileron = 1.0f;
                //_elevator = -Mathf.PI / 2 - hand.PalmNormal.Pitch; // Mathf.PI + hand.Direction.Pitch; //hand.Direction.Pitch;
                _elevator = Mathf.PI / 2 + hand.PalmNormal.Pitch; //Taranis reads elevator as +ve forwards (dive) and -ve backwards (climb)
                if (_elevator < -1.0f) _elevator = -1.0f;
                else if (_elevator > 1.0f) _elevator = 1.0f;
                //_rudder = hand.Direction.Yaw;
                //palm height seems to be 0 to 0.4 for about 40cm above the sensor, so looks like it's reading metres above sensor
                _throttle = (hand.PalmPosition.y-0.30f) * 5f; //centre on 30cm over sensor, 20cm either way
                if (_throttle < -1.0f) _throttle = -1.0f;
                else if (_throttle > 1.0f) _throttle = 1.0f;
                _grip = hand.GetFistStrength();
            }
        }
        UpdateSticksGraphics();
    }

    /** Updates the physics HandRepresentations. */
    protected virtual void OnFixedFrame(Frame frame)
    {
        if (frame != null)
        {
            //UpdateHandRepresentations(physicsHandReps, ModelType.Physics, frame);
        }
    }
}
