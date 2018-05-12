//
// Created by ivan on 10.05.2018..
//

#include "pe_header.h"
#include <inttypes.h>

BOOL IsValidPEHeader(uint8_t* baseAddress) {
    if (!baseAddress) return FALSE;
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)baseAddress;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return FALSE;
    PIMAGE_NT_HEADERS64 ntHeader = (PIMAGE_NT_HEADERS64)(baseAddress + dosHeader->e_lfanew);
    if (ntHeader->Signature != IMAGE_NT_SIGNATURE) return FALSE;
    PIMAGE_OPTIONAL_HEADER64 optionalHeader = (PIMAGE_OPTIONAL_HEADER64)&ntHeader->OptionalHeader;
    if (optionalHeader->Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC) return FALSE;
    return TRUE;
}

void* getPESectionDataByName(const char *sectionName) {

    uint8_t* baseAddress = GetModuleHandle(NULL);

    if (!IsValidPEHeader(baseAddress))
        return FALSE;

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)baseAddress;
    PIMAGE_NT_HEADERS64 ntHeader = (PIMAGE_NT_HEADERS64)((ULONG_PTR)dosHeader + dosHeader->e_lfanew);
    PIMAGE_FILE_HEADER fileHeader = &ntHeader->FileHeader;
    PIMAGE_OPTIONAL_HEADER64 optionalHeader = &ntHeader->OptionalHeader;

    uint8_t* firstSectionHeader = IMAGE_FIRST_SECTION(ntHeader);
    for (int i = 0; i < fileHeader->NumberOfSections; i++) {

        PIMAGE_SECTION_HEADER sectionHeader = (PIMAGE_SECTION_HEADER)(firstSectionHeader + i * sizeof(IMAGE_SECTION_HEADER));
        if (!strncmp((PCHAR)sectionHeader->Name, sectionName, IMAGE_SIZEOF_SHORT_NAME)) {
            void* dataPtr = baseAddress + sectionHeader->VirtualAddress;
            return dataPtr;
        }
    }
    return NULL;
}
