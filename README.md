# rmTouchToggler

This tool lets you toggle the touchpad by pressing both the left and the right button at once.

This is a answer to [this post](https://www.reddit.com/r/RemarkableTablet/comments/b48tbv/).

# Install

Copy the latest release binary (or compile it yourself using the [cross-compiler](https://remarkable.engineering/)) to the reMarkable.
It should be in the home directory (at `/home/root/touchToggler`).

Copy the file `touchToggler.service` into `/etc/system/systemd/`.

SSH into the reMarkable.

Run `chmod +x touchToggler`.

Start it using `systemctl start touchToggler` and stop it using `systemctl stop touchToggler`.

To enable it on autostart run `systemctl enable touchToggler`. To revert this run `systemctl disable touchToggler`.
