# AvConnect

... or how I programmed my own serial connector rather than try the macOS mobiflight alternatives.
This is a very basic plugin that allows you to connect some Mobiflight devices to XP directly,
without any external app.

## Limitations

- This will only work on macOS for now, mostly because I haven't tried to implement serial emulation
  for Linux or Windows.
- I built this so I could use my (excellent) Avionique GMC-710, so it basically only supports the
  types of inputs and outputs used by the GMC-710. Adding new types is probably relatively easy! I
  probably won't do it! Because I don't have a tonne of free time, and no hardware to test them!
- The ability to do maths on datarefs before setting outputs is... limited. Realistically, some
  limited scripting ability would be quite good there. See point above about my limited time :)
  
## Usage

1. Put the plugin in the `<X-Plane>/Resources/plugins` directory;
2. Once X-Plane is running and your device(s) connected, open the configuration window using the
   Plugins > AvConnect > Settings... menu;
3. Add a new device;
4. Select the serial port from the dropdown (re-scan if required);
4. Either query the various inputs and outputs from the device using the "Request Config" button,
   or add them manually;
5. Configure to your heart's content!
  
## Contributing

Feel free to fork and edit. That being said, due ton the aforementioned limited time, I very likely
might not have much time to review and integrate Pull Requests.
