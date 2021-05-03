"""
A bit of ASCII art...

 
 ___________         ___________ 
|     :     |   |   |           |
|     :     |   |   |           |
|-----O-----|   O   |     O     |
|     :     |   |   |           |
|_____:_____|   |   |___________|     
 
 -----O-----

"""
# http://ascii-table.com/ansi-escape-sequences-vt-100.php
# https://en.wikipedia.org/wiki/ANSI_escape_code

class TXViewer:
    def __init__(self):
        self.txText = [\
               " _________________       _________________ ",
               "|                 |     |                 |",
               "|                 |     |                 |",
               "|                 |     |                 |",
               "|                 |     |                 |",
               "|        x        |     |        x        |",
               "|                 |     |                 |",
               "|                 |     |                 |",
               "|                 |     |                 |",
               "|_________________|     |_________________|"
        ]

        #self.linelength=42 #length of one row in characters
        self.centreleft=(10,5) #centre point of left stick
        self.centreright=(34,5) #centre point of right stick
        self.multiplier=[8,4,4,8] #AETR multiplies stick [-1..+1] or [0..1] by this to map onto screen
        self.lastLeftPos=self.centreleft
        self.lastRightPos=self.centreright

        #cls
        #print(chr(27) + "[2J")
        print("\033c")
        #print graphic here
        #print("\033[0;0H",end="") #no newline
        for y in range(0,len(self.txText)):
            print("\033["+str(y+1)+";0H"+self.txText[y])
        #print("\033[1;15H?")

    ################################################################################

    def getCharAt(self, (x,y) ):
        #note x and y start at 1
        if (y>=1) and (y<=len(self.txText)) and (x>=1) and (x<=len(self.txText[y-1])):
            return self.txText[y-1][x-1]
        return ' '

    ################################################################################


    """
    All controls -1..+1 apart from throttle which is 0..1
    """
    def update(self,aileron,elevator,throttle,rudder):
        #limits check
        rudder=aileron
        aileron = max(-1,aileron)
        aileron = min(1,aileron)
        elevator = max(-1,elevator)
        elevator = min(1,elevator)
        throttle = max(0,throttle) #it's 0..1 remember
        throttle = min(1,throttle)
        rudder = max(-1,rudder)
        rudder = min(1,rudder)
        #left stick, throttle, rudder
        lx=int(rudder*self.multiplier[3] + self.centreleft[0])
        ly=int(self.centreleft[1] - 2*(throttle-0.5)*self.multiplier[2]) #remember 0..1
        #right stick, aileron, elevator
        rx=int(aileron*self.multiplier[0] + self.centreright[0])
        ry=int(elevator*self.multiplier[1] + self.centreright[1])
        #print space at last pos
        leftchar = self.getCharAt(self.lastLeftPos)
        print("\033["+str(self.lastLeftPos[1])+";"+str(self.lastLeftPos[0])+"H"+leftchar) #left
        rightchar = self.getCharAt(self.lastRightPos)
        print("\033["+str(self.lastRightPos[1])+";"+str(self.lastRightPos[0])+"H"+rightchar) #right
        #print O at new pos
        print("\033["+str(ly)+";"+str(lx)+"HO")
        print("\033["+str(ry)+";"+str(rx)+"HO")
        #set last pos
        self.lastLeftPos = (lx,ly)
        self.lastRightPos = (rx,ry)
        #and move cursor to below diagram
        print("\033[8;1H")

    ################################################################################
        

