using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System.IO;
using System.IO.Ports;

public class SerialCOM : MonoBehaviour
{
    //http://www.alanzucconi.com/2015/10/07/how-to-integrate-arduino-with-unity/

    public string portName="COM5"; //e.g. COM3
    public int baudRate = 9600; //115200;
    
    private SerialPort port;

    #region properties

    public bool isOpen
    {
        get { return (port!=null)&&(port.IsOpen); }
    }

    #endregion properties

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    /// <summary>
    /// Return a list of all currently active COM ports. this.portName should be set to one of them.
    /// </summary>
    /// <returns></returns>
    public static string[] listPorts()
    {
        string[] portNames = System.IO.Ports.SerialPort.GetPortNames();
        return portNames;
    }

    /// <summary>
    /// Open port with name this.portName and baud rate. Standard 8n1 no handshake.
    /// </summary>
    /// <param name="newPortName">The name of the new port to open. This is copied to this.portName in case you need to read it back.</param>
    /// <returns>True if the port opens</returns>
    public bool openPort(string newPortName)
    {
        this.portName = newPortName;
        port = new SerialPort(portName);
        port.BaudRate = baudRate;
        port.Parity = Parity.None;
        port.StopBits = StopBits.One;
        port.DataBits = 8;
        port.Handshake = Handshake.None;
        port.RtsEnable = true;

        //StreamWriter writer = new StreamWriter(port.BaseStream);

        port.Open();
        return port.IsOpen;
    }

    public void closePort()
    {
        port.Close();
        port = null;
    }

    public void flush()
    {
        port.BaseStream.Flush();
    }

    /// <summary>
    /// Send two bytes corresponding to an RC channel in ROGUE format using two bytes to encode channel and value.
    /// Channel is 0..15, value is -1..+1
    /// </summary>
    /// <param name="channel"></param>
    /// <param name="value"></param>
    public void sendROGUEData(int channel, float value)
    {
        //Encode two rogue format serial bytes and transmit - two bytes, one containig channel and high servo value, the other the low servo value.
        //Byte 1
        //channel (C) and upper 4 bits of value (H)
        //0CCC HHHH
        //Byte 2
        //lower value 7 bits (L)
        //1LLL LLLL
        //
        //NOTE: value (HHHHLLLLLLL) is signed, so +1023 to -1024

        int servo = (int)(1024 * value);
        if (servo < -1024) servo = -1024;
        else if (servo > 1023) servo = 1023;

        byte b1 = (byte)( (channel << 4) | ((servo>>7)&0x0f) );
        byte b2 = (byte)(0x80 | (servo&0x7f));

        byte[] buffer = new byte[] { b1, b2 };
        //byte[] buffer = new byte[] { 0x0, 0x8f };
        port.Write(buffer, 0, 2);
        //TEST: port.BytesToWrite
    }
}
