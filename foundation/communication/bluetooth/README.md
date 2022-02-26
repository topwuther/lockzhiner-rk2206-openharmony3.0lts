# Bluetooth<a name="EN-US_TOPIC_0000001148577119"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section161941989596)
-   [Constraints](#section119744591305)
-   [Usage](#section1312121216216)
    -   [Usage Guidelines](#section129654513264)

-   [Repositories Involved](#section1371113476307)

## Introduction<a name="section11660541593"></a>

The Bluetooth module provides APIs for accessing and using Bluetooth services, such as APIs for GATT operations, BLE advertising, and scan.

## Directory Structure<a name="section161941989596"></a>

```
/foundation/communication/bluetooth
├── interfaces         # APIs exposed externally
├── LICENSE            # Copyright notice file
```

## Constraints<a name="section119744591305"></a>

The Bluetooth module is compiled using the C language.

## Usage<a name="section1312121216216"></a>

Currently, only BLE-related APIs are available. APIs for different profiles, such as A2DP, AVRCP, and HFP will be provided in the future.

### Usage Guidelines<a name="section129654513264"></a>

-   Perform the following steps to enable the GATT server feature and start the GATT service:

```
/* Initialize the Bluetooth protocol stack. */
int InitBtStack(void);
int EnableBtStack(void);
/* Register an application with a specified appUuid. */
int BleGattsRegister(BtUuid appUuid);
/* Add a service. */
int BleGattsAddService(int serverId, BtUuid srvcUuid, bool isPrimary, int number);
/* Add a characteristic. */
int BleGattsAddCharacteristic(int serverId, int srvcHandle, BtUuid characUuid, int properties, int permissions);
/* Add a descriptor. */
int BleGattsAddDescriptor(int serverId, int srvcHandle, BtUuid descUuid, int permissions);
/* Start the GATT service. */
int BleGattsStartService(int serverId, int srvcHandle);
```

-   Perform the following steps to enable BLE advertising.

```
/* Set the data to advertise. */
int BleSetAdvData(int advId, const BleConfigAdvData *data);
/* Start advertising. */
int BleStartAdv(int advId, const BleAdvParams *param);
```

-   If you want to use scan capabilities, perform the following steps:

```
/* Set scan parameters. */
int BleSetScanParameters(int clientId, BleScanParams *param);
/* Start a scan. */
int BleStartScan(void);
```

## Repositories Involved<a name="section1371113476307"></a>

communication\_bluetooth

