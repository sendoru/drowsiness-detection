
# coding: utf-8

# In[ ]:


import pygame
import RPi.GPIO as GPIO

def select_alarm(result) :
    # most sleepy
    if result == 0:
        GPIO.output(11, 1)
        GPIO.output(13, 1)
        sound_alarm("/home/pi/Desktop/drowsiness-detection-master/power_alarm.wav")
    # moderately sleepy
    elif result == 1 :
        GPIO.output(11, 1)
        GPIO.output(13, 1)
        sound_alarm("/home/pi/Desktop/drowsiness-detection-master/nomal_alarm.wav")
    # slightly sleepy
    else :
        GPIO.output(11, 0)
        GPIO.output(13, 1)
        sound_alarm("/home/pi/Desktop/drowsiness-detection-master/short_alarm.mp3")

def sound_alarm(path) :
    try:
        pygame.mixer.init()
        pygame.mixer.music.load(path)
        pygame.mixer.music.play()
    except:
        pass

    

