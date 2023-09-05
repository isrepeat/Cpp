#pragma once
#include "BIOS.h"
#include "Conversion.h"
#include <Windows.h>
#include <sysinfoapi.h>
#include <iostream>
#include <string>
#include <span>


namespace {
    /*!
    * (c) Mircea Neacsu 2011. All rights reserved.
    * Code project: https://www.codeproject.com/Tips/5263343/How-to-Get-the-BIOS-UUID
    * Git: https://gist.github.com/neacsum/5199627fec4143c1e8d553d38dabed68
    */

    /*!
      SMBIOS Structure header as described at
      https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.3.0.pdf
      (para 6.1.2)
    */
    struct dmi_header
    {
        BYTE type;
        BYTE length;
        WORD handle;
        BYTE data[1];
    };

    /*
      Structure needed to get the SMBIOS table using GetSystemFirmwareTable API.
      see https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemfirmwaretable
    */
    struct RawSMBIOSData {
        BYTE  Used20CallingMethod;
        BYTE  SMBIOSMajorVersion;
        BYTE  SMBIOSMinorVersion;
        BYTE  DmiRevision;
        DWORD  Length;
        BYTE  SMBIOSTableData[1];
    };

    /*
      Get BIOS UUID in a 16 byte array.
      Return true if successful, 0 otherwise
    */
    bool biosuuid(BYTE* uuid)
    {
        bool result = false;

        RawSMBIOSData* smb = nullptr;
        BYTE* data;

        DWORD size = 0;

        // Get size of BIOS table
        size = GetSystemFirmwareTable('RSMB', 0, smb, size);
        smb = (RawSMBIOSData*)malloc(size);

        // Get BIOS table
        GetSystemFirmwareTable('RSMB', 0, smb, size);

        //Go through BIOS structures
        data = smb->SMBIOSTableData;
        while (data < smb->SMBIOSTableData + smb->Length)
        {
            BYTE* next;
            dmi_header* h = (dmi_header*)data;

            if (h->length < 4)
                break;

            //Search for System Information structure with type 0x01 (see para 7.2)
            if (h->type == 0x01 && h->length >= 0x19)
            {
                data += 0x08; //UUID is at offset 0x08

                // check if there is a valid UUID (not all 0x00 or all 0xff)
                bool all_zero = true, all_one = true;
                for (int i = 0; i < 16 && (all_zero || all_one); i++)
                {
                    if (data[i] != 0x00) all_zero = false;
                    if (data[i] != 0xFF) all_one = false;
                }
                if (!all_zero && !all_one)
                {
                    /* As off version 2.6 of the SMBIOS specification, the first 3 fields
                    of the UUID are supposed to be encoded on little-endian. (para 7.2.1) */
                    *uuid++ = data[3];
                    *uuid++ = data[2];
                    *uuid++ = data[1];
                    *uuid++ = data[0];
                    *uuid++ = data[5];
                    *uuid++ = data[4];
                    *uuid++ = data[7];
                    *uuid++ = data[6];
                    for (int i = 8; i < 16; i++)
                        *uuid++ = data[i];

                    result = true;
                }
                break;
            }

            //skip over formatted area
            next = data + h->length;

            //skip over unformatted area of the structure (marker is 0000h)
            while (next < smb->SMBIOSTableData + smb->Length && (next[0] != 0 || next[1] != 0))
                next++;
            next += 2;

            data = next;
        }
        free(smb);
        return result;
    }
}

namespace H {
    std::string GetBiosUuid() {
        BYTE uuid[16];
        if (biosuuid(uuid)) {
            std::string uuidStr(36, '\0');

            std::span<char> spanUuid = uuidStr;
            auto it = spanUuid.begin();
            const auto itEnd = spanUuid.end();

            it = HexByte(uuid[0], it, itEnd);
            it = HexByte(uuid[1], it, itEnd);
            it = HexByte(uuid[2], it, itEnd);
            it = HexByte(uuid[3], it, itEnd);
            *it++ = '-';
            it = HexByte(uuid[4], it, itEnd);
            it = HexByte(uuid[5], it, itEnd);
            *it++ = '-';
            it = HexByte(uuid[6], it, itEnd);
            it = HexByte(uuid[7], it, itEnd);
            *it++ = '-';
            it = HexByte(uuid[8], it, itEnd);
            it = HexByte(uuid[9], it, itEnd);
            *it++ = '-';
            it = HexByte(uuid[10], it, itEnd);
            it = HexByte(uuid[11], it, itEnd);
            it = HexByte(uuid[12], it, itEnd);
            it = HexByte(uuid[13], it, itEnd);
            it = HexByte(uuid[14], it, itEnd);
            it = HexByte(uuid[15], it, itEnd);

            return uuidStr;
        }

        return "";
    }
}