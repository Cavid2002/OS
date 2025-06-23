#include "../include/memory.h"
#include "../include/VGA.h"


static mmap_descriptor mdesc;


void init_mem_list()
{
    mdesc.mmap_list_size = (*(uint32_t*)MEM_LIST_ADDR);
    mdesc.mmap_list = (mmap_list_entry*)(MEM_LIST_ADDR + 4);
    for(int i = 0; i < mdesc.mmap_list_size; i++)
    {
        terminal_printf("\n0x%X %d %d\n", 
            mdesc.mmap_list[i].base_low,
            mdesc.mmap_list[i].lenght_low,
            mdesc.mmap_list[i].type);
    }
}


// void sort_mmap()
// {
//     int size = mdesc.mmap_list_size, min_id;
//     mmap_list_entry* arr = mdesc.mmap_list;
//     mmap_list_entry temp;
//     for(int i = 0; i < size; i++)
//     {
//         min_id = i;
//         for(int j = i + 1; j < size; j++)
//         {
//             if(arr[j].base < arr[min_id].base)
//             {
//                 min_id = i;
//             }
//         }

//         if(min_id == i) continue;
//         temp = arr[min_id];
//         arr[min_id] = arr[i];
//         arr[i] = temp;    
//     }
// }


// void filter_mmap()
// {
//     int size = mdesc.mmap_list_size, min_id;
//     mmap_list_entry* arr = mdesc.mmap_list;

//     for(int i = 0; i < size; i++)
//     {
//         switch (arr[i].type)
//         {
//         case MEM_BAD: case MEM_ACPI_NVS:
//             arr[i].type = MEM_RESERVED;
//             break;
//         case MEM_ACPI_REC: case MEM_USABLE: case MEM_RESERVED:
//             break;
//         default:
//             arr[i].type = MEM_RESERVED; 
//             break;
//         }
//     }
// }

// void mmap_combine()
// {
//     int size = mdesc.mmap_list_size, min_id;
//     mmap_list_entry* arr = mdesc.mmap_list;
//     mmap_list_entry* ptr1; mmap_list_entry* ptr2;
//     uint64_t end1, end2, new_base, new_lenght;
//     uint64_t max_end;
//     for(int i = 0; i < size - 1; i++)
//     {
//         ptr1 = arr + i;
//         ptr2 = arr + i + 1;

//         end1 = ptr1->base + ptr1->lenght;
//         end2 = ptr2->base + ptr2->base;

//         if(ptr2->base < end1 && ptr1->type == ptr2->type)
//         {
//             max_end = (end1 > end2) ? end1 : end2;
//             new_base = ptr1->base;
//             new_lenght = max_end - ptr1->base;

//             ptr1->base = new_base;
//             ptr2->lenght = new_lenght;

//             if(i + 2 < size)
//             {
                
//             }
            
//         }

//     }
// }