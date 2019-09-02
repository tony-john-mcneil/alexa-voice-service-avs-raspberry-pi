# Ubuntu Instructions for Alexa Voice Service RpiZeroW purchashed late 2018

This write-up is primarliy based on the guide here:
https://developer.amazon.com/docs/alexa-voice-service/register-a-product.html

_Downloaded: 2018-11-13-raspbian-stretch-lite.zip_

Get your microSD card connected to your linux machine.

_Usually by some sort of adaptor, note that if you're reconnecting an sdcard with existing pi setup
I have noticed it can be mounted in read-only mode, this happend to me when using a micro-sd to sd adaptor
(and no the slide lock wasn't in the wrong position.. perhaps it was faulty though)... anyhow you may
need to execute the following:_

_Check existing mount points:_

NOTE: sdX is usually sda or sdb, I'm using sdX here to prevent any cut'n'paste errors

```
mount
# snip example output... see the "ro" for readonly
/dev/sdX1 on /media/t/boot type vfat (ro,nosuid,nodev,relatime,uid=1000,gid=1000,fmask=0022,dmask=0022,codepage=437,iocharset=iso8859-1,shortname=mixed,showexec,utf8,flush,errors=remount-ro,uhelper=udisks2)
/dev/sdX2 on /media/t/rootfs type ext4 (ro,nosuid,nodev,relatime,data=ordered,uhelper=udisks2)
```

_Commands to remount in read/write mode:_

```
sudo mount -o remount,rw /dev/sdX1 /media/t/boot
sudo mount -o remount,rw /dev/sdX2 /media/t/rootfs
```

Follow instructions here for your preferred method, I'll document the "dd" method below:
_(reference instructions: https://www.raspberrypi.org/documentation/installation/installing-images/linux.md)_

Identify sdcard:

```sh
lsblk
```

Example output:

```
...snip..
sda                             8:0    1  29.7G  0 disk
├─sda1                          8:1    1  43.9M  0 part  /media/t/boot
└─sda2                          8:2    1  29.7G  0 part  /media/t/rootfs
...snip...
```

Unmount all mounted partitions:

```
umount /dev/sdX1
umount /dev/sdX2
```

Write the image to sdcard NOTE: use "sda" from above without the numbers for the partitions

```
sudo dd bs=4M if=2018-11-13-raspbian-stretch-lite.img of=/dev/sdX
```

Store example output for future checks and balances checking:

```
445+0 records in
445+0 records out
```

_Follow the headless instructions here (document some steps below in-case this link breaks in the future):
https://www.raspberrypi.org/documentation/configuration/wireless/headless.md

NOTE below we're accessing the root of the "boot" partition NOT the boot folder in the rootfs partition!
(this terminology can trip people up!)_

Wifi first:

```sh
sudo nano /media/t/boot/wpa_supplicant.conf
```

_/media/t above is where the sdcard got mounted to_

Example content (for normal Pi, PiZero variation in the next example) for configuring 2 networks, 1 on a home router and the other for a phone so
you can take the project mobile and show off:

```
country=AU
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1

network={
    ssid="the_1st_2.4-hz-wireless-network"
    psk="the_1st_2.4-hz-wireless-network_password"
    key_mgmt=WPA-PSK
    id_str="router"
    priority=1
}

network={
    ssid="the_2nd__2.4-hz-wireless-network"
    psk="the_2nd_2.4-hz-wireless-network_password"
    key_mgmt=WPA-PSK
    id_str="phone"
    priority=2
}

```

For the PiZero I needed the following variation:

```
country=AU
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
  
network={
        scan_ssid=1
        ssid="the_2.4-hz-wireless-network"
        psk="the_2.4-hz-wireless-network_password"
        proto=WPA
        key_mgmt=WPA-PSK 
}
```

*NOTE: you can try the wpa_supplicant file without scan_ssid and proto.. however I did need these at some stage for the pi zero*

Enable ssh access:

```sh
sudo touch /media/t/boot/ssh
```

SAFELY unmount/eject your sdcard

Put the sdcard in the pi and power up!

Check your routers dhcp table, for mine it's:
192.168.1.254 > login to admin interface > advanced > status > DHCP Table
(every router is different)

Once the dhcp allocated ip address has been confirmed for the pi, ssh in:

```sh
ssh pi@192.168.1.16
```

Default password for pi is: raspberry 

Now configure the pi with:

```ssh
sudo rasp-config
```

Update + upgrade system:

```sh
sudo apt update && sudo apt upgrade
```

Now download some helper scripts from github:

```sh
cd ~; \
wget https://raw.githubusercontent.com/alexa/avs-device-sdk/master/tools/Install/setup.sh; \
wget https://raw.githubusercontent.com/alexa/avs-device-sdk/master/tools/Install/genConfig.sh; \
wget https://raw.githubusercontent.com/alexa/avs-device-sdk/master/tools/Install/pi.sh
```

Sign in to AWS Develper Console: https://developer.amazon.com/avs/home.html#/avs/home

Create an AVS product, security profile + a client under: security profile > other devices and platforms

Downlod the config.json file for this client into the pi home directory.

This is a good time to make a backup image...

## Steps for making a backup image

shutdown the pi:

```sh
sudo shutdown -t now
```

(wait for the green light to stop flashing, then disconnect)

Remove the sd from pi and connect to computer 

Note the mounted folder and device (by running 'mount') and un-mount them e.g.

```sh
sudo umount /media/t/boot
sudo umount /media/t/rootfs
```

Make a backup (this will take a while):

```sh
sudo dd bs=4M if=/dev/sdX | gzip > ~/Projects/AWS/AlexaVoiceServicePi/backup-image-`date +%Y%m%d%H%M`.gz
```

sudo dd bs=4M oflag=direct status=progress if=/dev/sdb | gzip > ~/Projects/AWS/AlexaVoiceServicePi/backup-image-`date +%Y%m%d%H%M`.gz


Go have a coffee or beer, or to monitor the size of the backup being generated you can open a new terminal windows and execute:

```sh
du -h ~/Projects/AWS/AlexaVoiceServicePi/backup-image-*.gz
```

Notes:
_remove the human readable '-h' flag on the above command if you wish to see the size in bytes_
_you can also monitor the current disk usage with 'df -h ~/Projects/AWS/AlexaVoiceServicePi' command_

If you ever need to restore the backup the command will be (replace YYYYMMDDHHMI with your particular backup files date):

```sh
sudo gzip -dc ~/Projects/AWS/AlexaVoiceServicePi/backup-image-YYYYMMDDHHMI.gz | sudo dd bs=4M of=/dev/sdX
```

_Depending on your system the dd command will have options to show progress, on ubuntu 18.04 it's "status=progress". This
may differ on your system_

## Perform installs of extra software + build project

Setup the previoulsy downloaded config (prompts will wait for input during setup):

```sh
sudo bash setup.sh config.json -s 123456
```

Change ownership back to pi for everything:

_Again we look to modify the permissions to user:group = pi with the 'chown' command (I like to do this after running sudo 
as it sets user:groug root and this is bad from a security standpoint, so at minimum I change user:group to pi)_

```sh
sudo chown -R pi:pi ~pi
```

Start the sample app:
 
```sh
./startsample.sh
```

If you get some errors, so try and install additional software...

Here are some errors I saw:

```
2019-04-23 10:08:25.512 [  1] I sdkVersion: 1.12.1
configFile /home/pi/build/Integration/AlexaClientSDKConfig.json
Running app with log level: DEBUG9
2019-04-23 10:08:25.553 [  1] W Logger:debugLogLevelSpecifiedWhenDebugLogsCompiledOut:level=DEBUG9:

... snip ...

2019-04-23 10:08:27.834 [  1] E MediaPlayer:setupPipelineFailed:reason=createAudioSinkElementFailed,audioSinkElement=alsasink
2019-04-23 10:08:27.835 [  1] E MediaPlayer:initPlayerFailed:reason=setupPipelineFailed

(SampleApp:1426): GStreamer-CRITICAL **: gst_object_unref: assertion 'object != NULL' failed

(SampleApp:1426): GLib-CRITICAL **: g_main_context_find_source_by_id: assertion 'source_id > 0' failed
2019-04-23 10:08:27.836 [  1] W Logger:debugLogLevelSpecifiedWhenDebugLogsCompiledOut:level=DEBUG9:

WARNING: By default DEBUG logs are compiled out of RELEASE builds.
Rebuild with the cmake parameter -DCMAKE_BUILD_TYPE=DEBUG to enable debug logs.

2019-04-23 10:08:27.836 [  1] E RequiresShutdown:~RequiresShutdownFailed:reason=notShutdown,name=SpeakMediaPlayer
2019-04-23 10:08:27.836 [  1] C SampleApplication:Failed to create media player for speech!
2019-04-23 10:08:27.838 [  1] C SampleApplication:Failed to initialize SampleApplication
Failed to create to SampleApplication!

```

Attempt some extra software installs:

```sh
sudo apt install gstreamer1.0-tools gstreamer1.0-alsa gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly
```

Some more:

```sh
apt-get install alsa-oss alsaplayer mpg321 alsaplayer-alsa alsa-base
```

### Output from build:

```
[100%] Built target SampleApp

==============> SAVING CONFIGURATION FILE ==============

Completed generation of config file: /home/pi/build/Integration/tmp_AlexaClientSDKConfig.json

==============> FINAL CONFIGURATION  ==============

... snip ...

// Notes for logging
// The log levels are supported to debug when SampleApp is not working as expected.
// There are 14 levels of logging with DEBUG9 providing the highest level of logging and CRITICAL providing
// the lowest level of logging i.e. if DEBUG9 is specified while running the SampleApp, all the logs at DEBUG9 and
// below are displayed, whereas if CRITICAL is specified, only logs of CRITICAL are displayed.
// The 14 levels are:
// DEBUG9, DEBUG8, DEBUG7, DEBUG6, DEBUG5, DEBUG4, DEBUG3, DEBUG2, DEBUG1, DEBUG0, INFO, WARN, ERROR, CRITICAL.

// To selectively see the logging for a particular module, you can specify logging level in this json file.
// Some examples are:
// To only see logs of level INFO and below for ACL and MediaPlayer modules,
// -  grep for ACSDK_LOG_MODULE in source folder. Find the log module for ACL and MediaPlayer.
// -  Put the following in json:

// "acl":{
//  "logLevel":"INFO"
// },
// "mediaPlayer":{
//  "logLevel":"INFO"
// }

// To enable DEBUG, build with cmake option -DCMAKE_BUILD_TYPE=DEBUG. By default it is built with RELEASE build.
// And run the SampleApp similar to the following command.
// e.g. ./SampleApp /home/ubuntu/.../AlexaClientSDKConfig.json /home/ubuntu/KittAiModels/ DEBUG9"
 Completed Configuration/Build 
```

We can modify the startsample.sh script to place the DEBUG level at the end of the command ./SampleApp command.

If you encounter issues try searching the following troubleshooting guide:
https://github.com/alexa/avs-device-sdk/wiki/Troubleshooting-Guide

Barring a solution here, you can search the posts and comments here:
https://github.com/alexa/avs-device-sdk/issues

I'm guessing most people like me are going to hack together bit and peices of equiptment to get things going 
on a shoestring budget and will encounter various issues with audio configuration. I would expect nothing less
than a handfull of issues to keep you bashing your head for hours! If you're new to this then welcome to hacking
on linux, otherwise, welcome back!!

See raspberry pi audio config here:
https://www.raspberrypi.org/documentation/configuration/audio-config.md

For Asound configuration try and to use bits and peices suggested online, but always refer back to the official
doco as a point of truth (as people out there can suggest some real messy config and dirty hacks). Official Asoundrc
doco: https://alsa-project.org/wiki/Asoundrc

If you have a bluetooth speaker then the following page might help:
https://www.sigmdel.ca/michel/ha/rpi/bluetooth_02_en.html
(I ended up performing the bluetooth config process for a bluetooth speaker. At one stage I had a combined usb speaker / mic
hardware I salvaged from a skype speaker+mic device which I also had working but not documented here. My .asoundrc config for
this is down in the APPENDIX section at the bottom of this article)


```
mkdir ~/temp

cd ~/temp

wget http://www.kernel.org/pub/linux/bluetooth/bluez-5.49.tar.xz

cd bluez-5.49/

./configure --prefix=/usr --mandir=/usr/share/man --sysconfdir=/etc --localstatedir=/var --enable-experimental

make -j4

sudo make install

sudo adduser pi bluetooth

sudo cp /etc/dbus-1/system.d/bluetooth.conf /etc/dbus-1/system.d/bluetooth.conf.bak

sudo nano /etc/dbus-1/system.d/bluetooth.conf
```

Contents should be (blindly copied from the above mentioned bluetooth tutorial, I understand the bluetooth group permission at least)

```
<!-- This configuration file specifies the required security policies
     for Bluetooth core daemon to work. -->

<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>

  <!-- ../system.conf have denied everything, so we just punch some holes -->

  <policy user="root">
    <allow own="org.bluez"/>
    <allow send_destination="org.bluez"/>
    <allow send_interface="org.bluez.Agent1"/>
    <allow send_interface="org.bluez.MediaEndpoint1"/>
    <allow send_interface="org.bluez.MediaPlayer1"/>
    <allow send_interface="org.bluez.Profile1"/>
    <allow send_interface="org.bluez.GattCharacteristic1"/>
    <allow send_interface="org.bluez.GattDescriptor1"/>
    <allow send_interface="org.bluez.LEAdvertisement1"/>
    <allow send_interface="org.freedesktop.DBus.ObjectManager"/>
    <allow send_interface="org.freedesktop.DBus.Properties"/>
  </policy>

  <!-- allow users of bluetooth group to communicate -->
  <policy group="bluetooth">
    <allow send_destination="org.bluez"/>
  </policy>

  <policy at_console="true">
    <allow send_destination="org.bluez"/>
  </policy>

  <policy group="lp">
    <allow send_destination="org.bluez"/>
  </policy>

  <policy context="default">
    <deny send_destination="org.bluez"/>
  </policy>

</busconfig>
```

After changing the above config a reboot is needed:

```
sudo reboot
```

Next steps are to install and configure Bluetooth Audio ALSA Backend bluez-alsa: 

```
sudo apt install bluealsa
```

Open a bluetooth session:

```
bluetoothctl
```

Then scan for devices:

```
[bluetooth]# power on
[bluetooth]# scan on
```

If you're like me a fair few devices in your home will come up. They will all have a mac address you can use to inspect the device info e.g.

```
[bluetooth]# info 30:21:3E:31:C6:2B
```

Tip: Ensure your speaker is in discovery mode. The device might have some sort or id / model number to help identify it in the list. You can perform
a scan from your mobile phone to help you get some identifing information.

Once you know the mac of the speaker:

```
[bluetooth]# info 11:58:02:A4:02:40
[bluetooth]# pair 11:58:02:A4:02:40
[bluetooth]# trust 11:58:02:A4:02:40
[bluetooth]# connect 11:58:02:A4:02:40
[bluetooth]# quit
```

(I didn't have any issues here apart from my speaker coming in and out of visibility thus failed connection. I positioned the speaker accordingly to fix)

Now some alsa sound configuration is needed by creating and modifying .asoundrc

This is my .asoundrc file for a raspberry pi headless configuration for a bluetooth speaker and usb microphone:
(I did find some more post comments online to help with this config, however sorry I can't remember all the browser tabs I had open)

```
pcm.!default {
  type asym
  capture.pcm "mic"
  playback.pcm "speaker"
}
pcm.mic {
  type plug slave { pcm "hw:1,0" }
}
pcm.speaker {
  type plug
  slave.pcm {
    type bluealsa device "11:58:02:A4:02:40"
    profile "a2dp"
  }
}
```

GOAL is to be able to use arecord and aplay commands with no special command line args (config should all come from
.asoundrc so if you find a command that came from online posts that works for you, try translating into .asoundrc config.

Example test flow:

```
arecord test.wav
# press Ctrl-C to stop recording 
aplay test.wav
```

For good measure if things are working we can copy our config to the global location:

```
sudo cp /etc/asound.conf /etc/asound.conf_bak
cp ~/.asoundrc /etc/asound.conf
```

After all is configured you can try and run the sample app that was built earlier:

```
cd ~
./startsample.sh
```

Hopefully the app starts and then waits for you to speak and then answers accordingly!

Now make the application start after reboot / power up...
(Modified version of the instructions here: https://www.raspberrypi.org/documentation/linux/usage/systemd.md)

Create a file alexa-pi.service, with the contents:
(name the file anything you like and modify commands accordingly)

```
[Unit]
Description=Pi AWS Alexa Voice Service  
After=network.target network-online.target bluetooth.target

[Service]
Type=idle
ExecStart=/bin/bash /home/pi/startsample.sh
WorkingDirectory=/home/pi/build/SampleApp/src
User=pi

[Install]
WantedBy=multi-user.target
```

Copy to systemd service config area:

```
sudo cp ./alexa-pi.service /etc/systemd/system/
```

NOTE: Currently there is a bug starting the SampleApp as a background service. The workaround is to make the following modification
to ~/avs-device-sdk/SampleApp/src/UserInputManager.cpp
The new code is between START NEW CODE ... END NEW CODE (which effectivly disables the user keyboard input detection in the sample)


```
  SampleAppReturnCode UserInputManager::run() {
      bool userTriggeredLogout = false;
      m_interactionManager->begin();
      while (true) {
          // START NEW CODE
          std::this_thread::sleep_for(std::chrono::hours(1));
          continue;
          // END NEW CODE

          char x;
          if (!readConsoleInput(&x)) {
              break;
          }
```

Start the service:

```
sudo systemctl start alexa-pi.service
```


Next if things are working..

Talk with Alexa!
Say "Alexa", then ask "What time is it?"
Say "Alexa", then ask "What's the volume of a sphere?"
Say "Alexa", then say "How did the stock market do today?"
Say "Alexa", then ask "Where were you born?"
Say "Alexa", then say "How do you say friend in Russian?"

Now enable the service and reboot your pi to ensure the sample app starts:

```
sudo systemctl enable alexa-pi.service

Reboot:

```
sudo reboot
```

Fingers crossed, you should see lights flashing on your pi (and eventually the bluetooth connection sound if using bluetooth speaker). Shortly after this
the service should be ready to talk to.

Create some helper scripts (these serve as a nice reminder on how to start / status / stop the service when you come back in a few months):

file: ~/alexa-pi-start.sh

```
#!/bin/bash
sudo systemctl start alexa-pi
```

file: ~/alexa-pi-status.sh

```
#!/bin/bash
sudo systemctl status alexa-pi
```

file: ~/alexa-pi-stop.sh

```
#!/bin/bash
sudo systemctl stop alexa-pi
```

Make them executable:

```
chmod u+x alexa-pi-*.sh
```

NOTE***

At some point in the future the Wake Word Engine (WWE) from Sensory WILL expire and you'll no longer be able to say "Alexa".

The following error will be seen in the logs when running with debug logging:

```
SensoryKeywordDetector:initFailed:reason=allocatingNewSessionFailed,error=snsrNew(): License expired
```

Here is a related issue that's been logged: https://github.com/alexa/avs-device-sdk/issues/724

You have 2 options:

Option 1: Follow instructions for renewing the development licence here: https://github.com/Sensory/alexa-rpi#license
Option 2: Try the KITT.ai build instructions here: https://github.com/alexa/avs-device-sdk/wiki/Build-Options#kittai


After exploring we can move on to the Advanced Pi Tutorials here, starting with
"Indicate Device State with Sounds":
https://developer.amazon.com/docs/alexa-voice-service/indicate-device-state-with-sounds.html

Adding the device state indicator was pretty straight forward.


Adding lighting effects coorosponding to the Alexa state.
----------------------------------------------------------

I have connected an arduino to the PI via USB. The Arduino has individually addressable leds wired to it that are controlled
by the FastLED lib via serial calls to the Arduino. The arduino also has a temporary push button wired in to detect presses
of 3 seconds or longer, when they occur it writes to the serial port.

So in summary, incoming serial single character input codes are used to control lighting connected to the arduino, an outgoing 
serial single character code is used to signal the PI if the button is held down on the arduino (used for shutdown).

Here is the Arduino code for reference:
_TODO: I should expand of the whole setup (i.e. wiring and theory) but just referencing the code here for now._

```
#include <FastLED.h>
#include <Bounce2.h>

#define SHUTDOWN_BUTTON_PIN 2
#define SHUTDOWN_PIEZO_PIN 11
#define SHUTDOWN_BUTTON_INTERVAL_MILLIS 3000
#define SHUTDOWN_BUZZ_MILLIS 500
#define SHUTDOWN_BUZZ_TONE 1000
#define SHUTDOWN_TIMEOUT_MILLIS 10000
#define STATE_SHUTDOWN 'D'

#define NUM_RAIL_STRIPS 2
#define NUM_LEDS_PER_RAIL_STRIP 16

#define NUM_EYES_STRIPS 1
#define NUM_LEDS_PER_EYES_STRIP 3

#define LED_BRIGHTNESS 22
#define LED_ON_OFF_RND_RANGE 100

#define LED_PROCESSING_DELAY_MILLIS 125
#define LED_LISTEN_DELAY_MILLIS 250

#define RAIL_1_STRIP_PIN 12
#define RAIL_2_STRIP_PIN 13
#define EYES_1_STRIP_PIN 10

#define SATE_UNASSIGNED '-'

#define STATE_IDLE 'I'
#define NUM_IDLE_COLORS 2
CRGB::HTMLColorCode woprProcessingColors[NUM_IDLE_COLORS] = {CRGB::Red, CRGB::Orange};

#define STATE_ALEXA_ERROR 'E'
#define STATE_ALEXA_SPEAKING 'S'
#define LED_PULSE_DELAY_MILLIS 2
int pluseAdjustmentStepCount = 0;
int pulseAdjustmentDirection = -1; // start by adjusting down
CRGB speakingBaseColor = CRGB::Blue;
CRGB errorBaseColor = CRGB::Red;

#define STATE_ALEXA_THINKING 'T'
#define STATE_ALEXA_LISTENING 'L'
#define NUM_LISTEN_COLORS 3
CRGB::HTMLColorCode woprBlueColors[NUM_LISTEN_COLORS] = {CRGB::Blue, CRGB::Aqua, CRGB::Azure};

CRGB ledRail[NUM_RAIL_STRIPS][NUM_LEDS_PER_RAIL_STRIP];
CRGB ledEyes[NUM_EYES_STRIPS][NUM_LEDS_PER_EYES_STRIP];

char currentState;
char lastState;

// Instantiate a Bounce object for shutdown button
Bounce shutdownDebouncer = Bounce(); 

void setup() {
  // Setup the shutdown button with an internal pull-up and attach shutdownDebouncer with time interval:
  pinMode(SHUTDOWN_BUTTON_PIN,INPUT_PULLUP);
  shutdownDebouncer.attach(SHUTDOWN_BUTTON_PIN);
  shutdownDebouncer.interval(SHUTDOWN_BUTTON_INTERVAL_MILLIS);
  pinMode(SHUTDOWN_PIEZO_PIN, OUTPUT); // for shutdown sound
  
  FastLED.addLeds<NEOPIXEL, RAIL_1_STRIP_PIN>(ledRail[0], NUM_LEDS_PER_RAIL_STRIP);
  FastLED.addLeds<NEOPIXEL, RAIL_2_STRIP_PIN>(ledRail[1], NUM_LEDS_PER_RAIL_STRIP);
  FastLED.addLeds<NEOPIXEL, EYES_1_STRIP_PIN>(ledEyes[0], NUM_LEDS_PER_EYES_STRIP);
  LEDS.setBrightness(LED_BRIGHTNESS);
  currentState = SATE_UNASSIGNED;
  lastState = SATE_UNASSIGNED;

  Serial.begin(9600);
}

void loop() {
  handleSerial();
  handleShutdown();

  switch (currentState) {
    case STATE_IDLE:
      woprStandbyLights();
      break;
    case STATE_ALEXA_LISTENING:
      if (lastState != currentState) {
        alexaListenLights();
      }
      break;
    case STATE_ALEXA_THINKING:
      // set the lights for thinking
      alexaThinkingLights();
      break;
    case STATE_ALEXA_SPEAKING:
      if (lastState != currentState) {
        allLightsTo(speakingBaseColor);
      }
      alexaSpeakingLights();
      break;
    case STATE_ALEXA_ERROR:
      if (lastState != currentState) {
        allLightsTo(errorBaseColor);
      }
      alexaErrorLights();
      break;
    case SATE_UNASSIGNED:
      allLightsTo(CRGB::Black);
      break;
  }

  lastState = currentState;
}

void handleShutdown() {
  shutdownDebouncer.update();
  // Turn on the LED if either button is pressed :
  if (shutdownDebouncer.read() == HIGH) {
    tone(SHUTDOWN_PIEZO_PIN, SHUTDOWN_BUZZ_TONE);
    delay(SHUTDOWN_BUZZ_MILLIS);       
    noTone(SHUTDOWN_PIEZO_PIN);
    Serial.write(STATE_SHUTDOWN); // send shutdown command over serial
    delay(SHUTDOWN_TIMEOUT_MILLIS); // set a delay to wait for shutdown (i.e. back to normal if shutdown doesn't occur)
  }
}

void handleSerial() {
  while (Serial.available() > 0) {
    char incomingCharacter = Serial.read();
    switch (incomingCharacter) {
      case STATE_IDLE:
      case STATE_ALEXA_LISTENING:
      case STATE_ALEXA_THINKING:
      case STATE_ALEXA_SPEAKING:
      case STATE_ALEXA_ERROR:
        currentState = incomingCharacter;
        break;
      default:
        currentState = SATE_UNASSIGNED;
        break;
    }
  }
}

void alexaSpeakingLights() {
  // for each strip
  for (int x = 0; x < NUM_RAIL_STRIPS; x++) {
    // for each led in rail strip
    for (int i = 0; i < NUM_LEDS_PER_RAIL_STRIP; i++) {
      if (pulseAdjustmentDirection == -1) {
        ledRail[x][i] += CHSV(0, 1, 1);
      } else {
        ledRail[x][i] -= CHSV(0, 1, 1);
      }
    }
  }

  for (int x = 0; x < NUM_EYES_STRIPS; x++) {
    // for each led in eyes strip
    for (int i = 0; i < NUM_LEDS_PER_EYES_STRIP; i++) {
      if (pulseAdjustmentDirection == -1) {
        ledEyes[x][i] += CHSV(0, 1, 1);
      } else {
        ledEyes[x][i] -= CHSV(0, 1, 1);
      }
    }
  }
  
  pluseAdjustmentStepCount++;
  if (pluseAdjustmentStepCount >= 200) {
    pluseAdjustmentStepCount = 0;
    pulseAdjustmentDirection *= -1;
  }

  FastLED.show();
  FastLED.delay(LED_PULSE_DELAY_MILLIS);
}

void alexaThinkingLights() {
  // for each rail strip
  for (int x = 0; x < NUM_RAIL_STRIPS; x++) {
    // for each led in rail strip
    for (int i = 0; i < NUM_LEDS_PER_RAIL_STRIP; i++) {
      if (random8(100) > 40) {
        // switching current rail led on
        ledRail[x][i] = woprBlueColors[random8(NUM_LISTEN_COLORS)];
      } else {
        // switch rail led off
        ledRail[x][i] = CRGB::Black;
      }
    }
  }

  // for each eyes strip
  for (int x = 0; x < NUM_EYES_STRIPS; x++) {
    // for each led in eyes strip
    for (int i = 0; i < NUM_LEDS_PER_EYES_STRIP; i++) {
      if (random8(100) > 40) {
        // switching current eyes led on
        ledEyes[x][i] = woprBlueColors[random8(NUM_LISTEN_COLORS)];
      } else {
        // switch eyes led off
        ledEyes[x][i] = CRGB::Black;
      }
    }
  }
  
  FastLED.show();
  FastLED.delay(LED_PROCESSING_DELAY_MILLIS);
}

void alexaErrorLights() {
  // for each strip
  for (int x = 0; x < NUM_RAIL_STRIPS; x++) {
    // for each led in rail strip
    for (int i = 0; i < NUM_LEDS_PER_RAIL_STRIP; i++) {
      if (pulseAdjustmentDirection == -1) {
        ledRail[x][i] += CHSV(0, 1, 1);
      } else {
        ledRail[x][i] -= CHSV(0, 1, 1);
      }
    }
  }

  for (int x = 0; x < NUM_EYES_STRIPS; x++) {
    // for each led in eyes strip
    for (int i = 0; i < NUM_LEDS_PER_EYES_STRIP; i++) {
      if (pulseAdjustmentDirection == -1) {
        ledEyes[x][i] += CHSV(0, 1, 1);
      } else {
        ledEyes[x][i] -= CHSV(0, 1, 1);
      }
    }
  }
  
  pluseAdjustmentStepCount++;
  if (pluseAdjustmentStepCount >= 200) {
    pluseAdjustmentStepCount = 0;
    pulseAdjustmentDirection *= -1;
  }

  FastLED.show();
  FastLED.delay(LED_PULSE_DELAY_MILLIS);
}

void alexaListenLights() {
  // for each rail strip
  for (int x = 0; x < NUM_RAIL_STRIPS; x++) {
    // for each led in rail strip
    for (int i = 0; i < NUM_LEDS_PER_RAIL_STRIP; i++) {
      // switching current rail led on
      ledRail[x][i] = woprBlueColors[random8(NUM_LISTEN_COLORS)];
    }
  }

  // for each eyes strip
  for (int x = 0; x < NUM_EYES_STRIPS; x++) {
    // for each led in eye strip
    for (int i = 0; i < NUM_LEDS_PER_EYES_STRIP; i++) {
      // switching current eyes led on
      ledEyes[x][i] = woprBlueColors[random8(NUM_LISTEN_COLORS)];
    }
  }
  
  FastLED.show();
}

void woprStandbyLights() {
  // for each rail strip
  for (int x = 0; x < NUM_RAIL_STRIPS; x++) {
    // for each led in rail strip
    for (int i = 0; i < NUM_LEDS_PER_RAIL_STRIP; i++) {
      if (random8(100) > 40) {
        // switching current rail led on
        ledRail[x][i] = woprProcessingColors[random8(NUM_IDLE_COLORS)];
      } else {
        // switch rail led off
        ledRail[x][i] = CRGB::Black;
      }
    }
  }

  // for each eyes strip
  for (int x = 0; x < NUM_EYES_STRIPS; x++) {
    // for each led in eyes strip
    for (int i = 0; i < NUM_LEDS_PER_EYES_STRIP; i++) {
      if (random8(100) > 40) {
        // switching current eyes led on
        ledEyes[x][i] = woprProcessingColors[random8(NUM_IDLE_COLORS)];
      } else {
        // switch eyes led off
        ledEyes[x][i] = CRGB::Black;
      }
    }
  }
  
  FastLED.show();
  FastLED.delay(LED_PROCESSING_DELAY_MILLIS);
}

void allLightsTo(CRGB color) {
  // for each rail strip
  for (int x = 0; x < NUM_RAIL_STRIPS; x++) {
    fill_solid(ledRail[x], NUM_LEDS_PER_RAIL_STRIP, color);
  }

  // for each eyes strip
  for (int x = 0; x < NUM_EYES_STRIPS; x++) {
    fill_solid(ledEyes[x], NUM_LEDS_PER_EYES_STRIP, color);
  }
  FastLED.show();
}

```

On the raspberry pi I have modified: ~/avs-device-sdk/SampleApp/src/UIManager.cpp 

Adding the system shell calls to send data to the arduino connected via serial, here is 
the full printState() method after modification:

```
void UIManager::printState() {
    if (m_connectionStatus == avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED) {
        ConsolePrinter::prettyPrint("Client not connected!");
        system("echo -n 'E' > /dev/serial/by-id/usb-Freetronics_Pty_Ltd_Leostick-if00");
    } else if (m_connectionStatus == avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::PENDING) {
        ConsolePrinter::prettyPrint("Connecting...");
    } else if (m_connectionStatus == avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::CONNECTED) {
        switch (m_dialogState) {
            case DialogUXState::IDLE:
                ConsolePrinter::prettyPrint("Alexa is currently idle!");
    system("echo -n 'I' > /dev/serial/by-id/usb-Freetronics_Pty_Ltd_Leostick-if00");
                return;
            case DialogUXState::LISTENING:
                ConsolePrinter::prettyPrint("Listening...");
    system("echo -n 'L' > /dev/serial/by-id/usb-Freetronics_Pty_Ltd_Leostick-if00");
                return;
            case DialogUXState::EXPECTING:
                ConsolePrinter::prettyPrint("Expecting...");
    system("echo -n 'U' > /dev/serial/by-id/usb-Freetronics_Pty_Ltd_Leostick-if00");
                return;
            case DialogUXState::THINKING:
                ConsolePrinter::prettyPrint("Thinking...");
    system("echo -n 'T' > /dev/serial/by-id/usb-Freetronics_Pty_Ltd_Leostick-if00");
                return;
            case DialogUXState::SPEAKING:
                ConsolePrinter::prettyPrint("Speaking...");
    system("echo -n 'S' > /dev/serial/by-id/usb-Freetronics_Pty_Ltd_Leostick-if00");
                return;
            /*
             * This is an intermediate state after a SPEAK directive is completed. In the case of a speech burst the
             * next SPEAK could kick in or if its the last SPEAK directive ALEXA moves to the IDLE state. So we do
             * nothing for this state.
             */
            case DialogUXState::FINISHED:
                return;
        }
    }   
}

```

The following helper shell script has been added to the raspberry pi to handle graceful shutdown when the arduino
button is held for 3+ seconds and it sends a serial command:

Script name: arduino-read-serial-shutdown.sh

Contents:

```
#!/bin/bash

ARDUINO_SERIAL="/dev/serial/by-id/usb-Freetronics_Pty_Ltd_Leostick-if00"

while read -N1 INPUT < $ARDUINO_SERIAL; do
  if [ "$INPUT" == "D" ]
  then
    sudo shutdown -t now 
  fi  
done

```

Ensure it's executable:

```
chmod u+x arduino-read-serial-shutdown.sh
```

Add a call to this script to the start of the "startsample.sh" script (note the trailing "&" after the call sends to the background).
Here is the modified "startsample.sh" script with the added call to "arduino-read-serial-shutdown.sh":

```
./arduino-read-serial-shutdown.sh&

cd "/home/pi/build/SampleApp/src"

./SampleApp "/home/pi/build/Integration/AlexaClientSDKConfig.json" "/home/pi/third-party/alexa-rpi/models"

```

Alternativly you can configure this script to be another service that starts at an earlier stage, which is what I've now done
on my setup as I was tinkering with my WiFi settings on the Pi and go into an invalid state where I could not login to perform
a safe shutdown, therefore I had to do an unsafe shutdown by pulling the power. If you have a standalone service you can perform
a safe shutdown regardless of being connected to a WiFi network.



Using pi -> arduino serial communication via the boost c++ library to control the arduino throughout the avs lifecycle.
-------------------------------------------------------------------------------------------------------------------------

There are many libraries you can choose from for Raspberry Pi serial communication to arduino boards,
and the libray I have chosen for this task is the popular Boost library https://www.boost.org/

_There is another lib that looks tempting called OpenFrameworks which has a specific Arduino Firmata communication
component: https://openframeworks.cc///documentation/communication/ofArduino/ however I have chosen boost as it 
seems to offer the most seamless install process via an apt package and as far as I can see OpenFrameworks is a 
manual install with configure and make which will have to be tailored for the Raspberry Pi.
(If you interested in using OpenFrameworks then see this page for install instructions:
https://openframeworks.cc/setup/linux-install/)_

Boost install steps:

```
sudo apt install libboost-all-dev
```

TODO: supply the serial communication sketch for arduino
TODO: supply a test c++ program for testing the serial communication

The following guide has been adapted to use boost communicating with an arduino via serial:
[Indicate Device State with LEDs](https://developer.amazon.com/docs/alexa-voice-service/indicate-device-state-with-leds.html)

NOTE: I decided to go with simple system shell calls to read and write to the serial port instead of the boost serial 
communication... so cutting this section short for now.



APPENDIX ~/.asoundrc config for a usb speak + mic combination:
-----------------------------------------------------------------

```
pcm.dsnooper {
    type dsnoop
    ipc_key 816357492
    ipc_key_add_uid 0
    ipc_perm 0666
    slave {
        pcm "hw:1,0"
        channels 1
    }
}
pcm.dmixer {
    type dmix
    ipc_key 1024
    slave {
        pcm "hw:1,0"
        period_time 0
        period_size 1024
        buffer_size 8192
        rate 44100
    }

    }
pcm.!default {
        type asym
        playback.pcm {
                type plug
                slave.pcm "dmixer"
        }
        capture.pcm {
                type plug
                slave.pcm "dsnooper"
        }
}

ctl.!default {
  type hw           
  card 1
}

```

APPENDIX changes to include c++ boost libraries
----------------------------------------------------------
File: avs-device-sdk/SampleApp/src/CMakeLists.txt

Content at end of file:
target_link_libraries(SampleApp "-lboost_system")


