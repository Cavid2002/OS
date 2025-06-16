#include "../include/memory.h"


static mmap_descriptor mdesc;


void init_mem_list()
{
    mdesc.mmap_list_size = (*(uint32_t*)MEM_LIST_ADDR);
    mdesc.mmap_list = (mmap_list_entry*)(MEM_LIST_ADDR + 4);
}


void sort_mmap()
{
    int size = mdesc.mmap_list_size, min_id;
    mmap_list_entry* arr = mdesc.mmap_list;
    mmap_list_entry temp;
    for(int i = 0; i < size; i++)
    {
        min_id = i;
        for(int j = i + 1; j < size; j++)
        {
            if(arr[j].base < arr[min_id].base)
            {
                min_id = i;
            }
        }

        if(min_id == i) continue;
        temp = arr[min_id];
        arr[min_id] = arr[i];
        arr[i] = temp;    
    }
}


void filter_mmap()
{
    int size = mdesc.mmap_list_size, min_id;
    mmap_list_entry* arr = mdesc.mmap_list;

    for(int i = 0; i < size; i++)
    {
        switch (arr[i].type)
        {
        case MEM_BAD: case MEM_ACPI_NVS:
            arr[i].type = MEM_RESERVED;
            break;
        case MEM_ACPI_REC: case MEM_USABLE: case MEM_RESERVED:
            break;
        default:
            arr[i].type = MEM_RESERVED; 
            break;
        }
    }
}

void mmap_combine()
{
    int size = mdesc.mmap_list_size, min_id;
    mmap_list_entry* arr = mdesc.mmap_list;

    for(int i = 1; i < size; i++)
    {
        mmap_list_entry* ptr1 = arr + i - 1;
        mmap_list_entry* ptr2 = arr + i;
        
        uint64_t end1 = ptr1->base + ptr1->lenght;
        uint64_t end2 = ptr2->base + ptr2->lenght;


        if(end1 > ptr2->base && ptr1->type == ptr2->type)
        {
            ptr1->lenght = ptr2->base - ptr1->base;
            ptr2->lenght = (end1 > end2) ? end1 : end2;
        }

    }
}