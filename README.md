# SEAMcaller

## Description

**SEAMcaller** is a Linux Kernel Module (LKM) designed for VMM-side analysis of Intel TDX (Trusted Domain eXtensions). 
It allows researchers and developers to observe, modify, and experiment with SEAM calls issued by the VMM during TD creation, build, and runtime TDX APICalls on **NON-production TDX systems**.

**Note:** This LKM is intended to be run on a TDX-supported hardware platform (TDX Server). For safety and legal considerations, please refer to the **Disclaimer** section before use.

## Features

- **SEAM Call Monitoring**  
 Hooks into SEAM calls using `kprobes` to observe call sequences from the VMM to the TDX module.  

- **Argument Manipulation**  
 Modify SEAM call arguments on-the-fly to test different behaviors in the TDX module.  

- **Arbitrary SEAM Calls**  
 Issue custom SEAM calls to the TDX module for experimentation and testing purposes.  

- **Flexible & Customizable**  
 Designed to be easily adapted for research and analysis of VMM-TDX interactions.

## Disclaimer

SEAMcaller is provided for **research and educational purposes only**. Use at your own risk. 
The authors **do not assume any liability** for damages, data loss, or security incidents resulting from its use. 
The tool is intended for controlled lab environments and **should not be used on production systems**.
