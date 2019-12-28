# rmButtonToggler

This tool lets you toggle all button input by pressing the left and right buttons together.

## IMPORTANT
Blocking all buttons also **includes** the **power/wakeup** button!  
Should the device enter sleep mode, you either need to perform the button combination again (it'll work even when sleeping) or restart the device by holding the power button for about 10 seconds (that will still work). Otherwise you won't be able to get the device out of sleep.

**DISCLAIMER:** I won't be liable for any harm done using this software.

# Install

Copy the latest release binary (or compile it yourself using the [cross-compiler](https://remarkable.engineering/)) to the reMarkable.
It should be in the home directory (at `/home/root/button_toggler`).

Copy the file `button_toggler.service` into `/etc/systemd/system/`.

SSH into the reMarkable.

Run `chmod +x button_toggler`.

Start it using `systemctl start button_toggler` and stop it using `systemctl stop button_toggler`.

To enable it on autostart run `systemctl enable button_toggler`. To revert this run `systemctl disable button_toggler`.
