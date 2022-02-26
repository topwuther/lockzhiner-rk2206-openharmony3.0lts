# selinux\_policy\_standard<a name="EN-US_TOPIC_0000001100252616"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section161941989596)
-   [Usage](#section119744591305)
-   [Repositories Involved](#section1371113476307)

## Introduction<a name="section11660541593"></a>

This repository stores the SELinux policy files required by each subsystem of an OpenHarmony standard system and the script files for compiling these SELinux policy files.

## Directory Structure<a name="section161941989596"></a>

```
/utils/system/selinux_policy_standard
├── account                     # Stores the SELinux policy files for the Account subsystem. The directory structure of other subsystems is the same as this one.
│   └── system
│       └── common              # Stores the SELinux policy files to be compiled into the system image.
│   └── vendor
│       └── common              # Stores the SELinux policy files to be compiled into the vendor image. If no such policy files are configured, this directory does not exist.
│   └── public                  # Stores the SELinux policy files to be compiled into the system and vendor images. If no such policy files are configured, this directory does not exist.
│   └── property_trustlist      # Stores the property_trustlist policy files to be compiled into the system image. If no such policy files are configured, this directory does not exist.
│   └── policy.mk               # Represents the makefile, which is associated with all the policy files for this subsystem.
├── appexecfwk                  # Stores the SELinux policy files for the Application Framework subsystem.
├── communication               # Stores the SELinux policy files for the Intelligent Soft Bus subsystem.
├── distributedschedule         # Stores the SELinux policy files for the Distributed Scheduler subsystem.
├── graphic                     # Stores the SELinux policy files for the Graphic subsystem.
├── hdf                         # Stores the SELinux policy files for the Hardware Driver Foundation (HDF) subsystem.
├── hiviewdfx                   # Stores the SELinux policy files for the DFX subsystem.
├── kernel                      # Stores the SELinux policy files for the Kernel subsystem.
├── miscservices                # Stores the SELinux policy files for the Misc Services subsystem.
├── multimedia                  # Stores the SELinux policy files for the Multimedia subsystem.
├── multimodalinput             # Stores the SELinux policy files for the Multimodal Input subsystem.
├── startup                     # Stores the SELinux policy files for the Startup subsystem.
├── telephony                   # Stores the SELinux policy files for the Telephony Service subsystem.
├── udevd                       # Stores the SELinux policy files for the udev module of the Multimodal Input subsystem.
├── uinput                      # Stores the SELinux policy files for the uinput module of the Multimodal Input subsystem.
├── updater                     # Stores the SELinux policy files for the Update subsystem.
├── ...                         # Stores the SELinux policy files for new subsystems.
├── ohos_policy.mk              # Represents the makefile, which is associated with the policy files for all subsystems in the current directory.
```

## Usage<a name="section119744591305"></a>

You can create SELinux policy files and script files for a new subsystem in corresponding directories that are determined by the image into which the SELinux policy files are compiled. When adding a new SELinux policy file, place it in the right directory. The structure of the directories for compiling SELinux policy files is as follows \(for details, see the  **ohos\_policy.mk**  file in the root directory\):

```
/utils/system/selinux_policy_standard
├── NEW                         # Stores the SELinux policy files for a new subsystem.
│   └── system
│       └── common              # Stores the SELinux policy files to be compiled into the system image.
│   └── vendor
│       └── common              # Stores the SELinux policy files to be compiled into the vendor image.
│   └── public                  # Stores the SELinux policy files to be compiled into the system and vendor images.
│   └── property_trustlist      # Stores the property_trustlist policy files to be compiled into the system image.
│   └── policy.mk               # Represents the makefile.
```

## Repositories Involved<a name="section1371113476307"></a>

Security subsystem

**hmf/utils/selinux\_policy\_standard**

