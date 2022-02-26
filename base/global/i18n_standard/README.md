# i18n<a name="EN-US_TOPIC_0000001101364976"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section1464106163817)
-   [Constraints](#section1718733212019)
-   [Usage](#section894511013511)
-   [Repositories Involved](#section15583142420413)

## Introduction<a name="section11660541593"></a>

The i18n module provides a wide array of internationalization \(i18n\) APIs for implementing functions such as date and time formatting.

## Directory Structure<a name="section1464106163817"></a>

The directory structure for the i18n module is as follows:

```
/base/global/
├── i18n_standard           # Code repository for the i18n framework
│   ├── frameworks          # Core code of the i18n framework
│   ├── interfaces          # APIs
│   │   ├── js              # JavaScript APIs
│   │   └── native          # Native APIs
```

## Constraints<a name="section1718733212019"></a>

**Development language**: JavaScript

**Language, script, and country codes**: The supported language must be represented by a two- or three-letter code defined in the ISO 639 standard; the supported script must be represented by a four-letter code defined in the ISO 15924 standard; the supported country must be represented by a two-letter code defined in the ISO 3166 standard.

## Usage<a name="section894511013511"></a>

Change the date and time formats \(such as the sequence of year, month, and day, month and week names, and 12-hour or 24-hour system\) following the system settings to adapt to the cultural habits of users in different locales. For details, see the API reference. The sample code is as follows:

```
const date = new Date(2021, 11, 17, 3, 24, 0); // Create a Date object containing date and time information.
fmt = new Intl.DateTimeFormat('en-US') // Create a DateTimeFormat instance.
console.log(fmt.format(date)); // Format the date and time by using the DateTimeFormat instance.
```

## Repositories Involved<a name="section15583142420413"></a>

Globalization subsystem

**global\_i18n\_standard**

global\_resmgr\_standard

