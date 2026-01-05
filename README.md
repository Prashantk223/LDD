A low-level Linux character device driver that exposes a pseudo device file interface to user space.
The driver supports read/write operations, device tree probing, and multiple device instances using the Linux device model.

• Implemented a Linux character device driver providing standard file operations (open, read, write, release) for user-space interaction.
• Created pseudo device nodes and registered devices dynamically, enabling multiple independent instances of the driver.
• Integrated the driver with the Linux device tree for probe and initialization using platform driver mechanisms.
• Managed per-device data structures to support multiple instances and safe concurrent access.
• Focused on clean driver architecture, error handling, and adherence to Linux kernel coding pra
