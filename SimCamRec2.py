#imports that are requited to use the camera on Rpi
from picamera.array import PiRGBArray
from picamera import PiCamera
import time

#OpenCV required imports 
import cv2
import numpy as np

#I/O 
import RPi.GPIO as GPIO

#command to open openCV dir: cd /usr/local/lib/python3.5/site-packages/
#working dir: cd ~/Documents/CS122A/SimCamRec.py /usr/local/lib/python3.5/site-packages/
#move working file to open cv: sudo cp ~/Documents/CS122A/SimCamRec2.py /usr/local/lib/python3.5/site-packages/

#globals
# Set the LED GPIO number
LEDB = 21
LEDR = 26
LEDG = 20

def readimagePurp(img):
        #check if the argument is a string to see if it is a file vs a video capture
    if(isinstance(img, str)):
        image = cv2.imread(img, 1)
    else:
        image = img

    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    
    #cv2.imshow("hsv", hsv) [123,  94,  53] [143, 114, 133]
    lower_purple = np.array([119,  98,  34])
    upper_purple = np.array([149, 139, 215])

    mask = cv2.inRange(hsv, lower_purple, upper_purple)
    return mask


def readimageRed(img):
        #check if the argument is a string to see if it is a file vs a video capture
    if(isinstance(img, str)):
        image = cv2.imread(img, 1)
    else:
        image = img

    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    
    #cv2.imshow("hsv", hsv)
    lower_red = np.array([1,183,44])
    upper_red = np.array([14,236,295])

    mask = cv2.inRange(hsv, lower_red, upper_red)
    return mask

def readimageBlue(img):
    #check if the argument is a string to see if it is a file vs a video capture
    if(isinstance(img, str)):
        image = cv2.imread(img, 1)
    else:
        image = img

    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    
    #cv2.imshow("hsv", hsv) [97,109,8]) [118,225,201]
    lower_blue = np.array([97,109,8])
    upper_blue = np.array([124,208,131])

    mask = cv2.inRange(hsv, lower_blue, upper_blue)
    return mask

def readimageGreen(img):
        #check if the argument is a string to see if it is a file vs a video capture
    if(isinstance(img, str)):
        image = cv2.imread(img, 1)
    else:
        image = img

    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    
    #cv2.imshow("hsv", hsv)
    lower_green = np.array([66,205,25])
    upper_green = np.array([87,268,164])

    mask = cv2.inRange(hsv, lower_green, upper_green)
    return mask

def genImageCheck(img, lower, upper):
    #check if the argument is a string to see if it is a file vs a video capture
    if(isinstance(img, str)):
        image = cv2.imread(img, 1)
    else:
        image = img

    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    
    #cv2.imshow("hsv", hsv)

    mask = cv2.inRange(hsv, lower, upper)
    return mask

def getImg():
    cap = cv2.VideoCapture(1)

    # Capture frame-by-frame
    ret, frame = cap.read()

    # Our operations on the frame come here
    img = cv2.cvtColor(frame, 1)

    # Display the resulting frame
    #cv2.imshow('frame',img)
    cv2.waitKey(1)
    # When everything done, release the capture
    cap.release()
    cv2.destroyAllWindows()
    return img
    
def piCam():
	# initialize the camera and grab a reference to the raw camera capture
	camera = PiCamera()
	camera.resolution = (640, 480)
	camera.framerate = 32
	rawCapture = PiRGBArray(camera, size=(640, 480))
	 
	# allow the camera to warmup
	time.sleep(0.1)
	
	return rawCapture, camera
	
def SetGPIO():
	# Set the GPIO mode
	GPIO.setmode(GPIO.BCM)
	GPIO.setwarnings(False)
	# Set the LED GPIO pin as an output
	GPIO.setup(LEDB, GPIO.OUT)
	GPIO.setup(LEDR, GPIO.OUT)
	GPIO.setup(LEDG, GPIO.OUT)
	
	return GPIO
	
def setRGB(rgb, GPIO):
	GPIO.output(LEDR,False)
	GPIO.output(LEDG,False)
	GPIO.output(LEDB,False)
	
	if(rgb == "red"):
		GPIO.output(LEDR,True)	
	elif(rgb == "blue"):
		GPIO.output(LEDB,True)
	elif(rgb == "green"):
		GPIO.output(LEDG,True)
	elif(rgb == "purp"):
		GPIO.output(LEDR,True)
		GPIO.output(LEDB,True)
	else:
		GPIO.output(LEDR,False)
		GPIO.output(LEDB,False)
		GPIO.output(LEDB,False)

	
if __name__ == '__main__':
    filename = "green2.jpg"
    lower_green = np.array([66,205,25])
    upper_green = np.array([87,268,164])

    lower_test = np.array([ 65, 20, -23])
    upper_test = np.array([85,40,57])
    
    lower_orange = np.array([ -4, 183, 125])
    upper_orange = np.array([ 16, 203, 205])
    
    GPIO = SetGPIO()
	
	# Methods to test program on a pc
    #capture = getImg()
    #filename = capture
    
    # Method to test program on Rpi
    rawCapture, camera = piCam()
    
	# capture frames from the camera
    for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
		# grab the raw NumPy array representing the image, then initialize the timestamp
		# and occupied/unoccupied text
		image = frame.array
		filename = image
		#	 show the frame
		#cv2.imshow("Frame", image)
		
		mask1 = readimageBlue(filename)
		mask2 = readimagePurp(filename)
		mask3 = readimageRed(filename)
		mask4 = readimageGreen(filename)
		mask5 = genImageCheck(filename,lower_test, upper_test)
		mask6 = genImageCheck(filename,lower_test, upper_test)
				

		#x4 = cv2.countNonZero(mask)
		
		x1 = cv2.countNonZero(mask1)
		x2 = cv2.countNonZero(mask2)
		x3 = cv2.countNonZero(mask3)
		x4 = cv2.countNonZero(mask4)
		x5 = cv2.countNonZero(mask5)
		x6 = cv2.countNonZero(mask5)

		print("x1 ", x1, "x2 ",x2, "x3 ", x3, "x4 " , x4, "x5 ", x5)
		
		if(x1 > 3000):
			print("This is a blue dice")
			setRGB("blue", GPIO)
			
		elif(x2>200):
			print("This is a purple dice")
			setRGB("purp", GPIO)
		elif(x3>3000):
			print("This is a red dice")
			setRGB("red", GPIO)
		elif(x4>3000):
			print("This is a green dice")
			setRGB("green", GPIO)
		#elif(x5>2000):
		#	print("black")
		#	setRGB("blue", GPIO)
		#	time.sleep(0.01)
		#	setRGB("red", GPIO)
		#	time.sleep(0.01)
		#	setRGB("effin", GPIO)
		elif(x6 > 3000):
			print("This is a orange dice")
			for i in range(0,100):
				setRGB("green", GPIO)
				time.sleep(0.0027)
				setRGB("red", GPIO)
				time.sleep(0.003)
				setRGB("effin", GPIO)					
		else:
			print("I just don't know, sorry.")
			setRGB("effin", GPIO)
			
		#cv2.imshow("mask", mask1)
				
		key = cv2.waitKey(1) & 0xFF
	 
		# clear the stream in preparation for the next frame
		rawCapture.truncate(0)
	 
		# if the `q` key was pressed, break from the loop
		if key == ord("q"):
			break
	#cv2.waitKey(0)
