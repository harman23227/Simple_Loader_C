#include "loader.h"
#include <stdbool.h>
//Declaring all the global variables for file header , program header , file pointer, virtual memory,minimum entry point
int i ,minentry;
void * virtual_mem = NULL;
Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
FILE * file;



/*
 * release memory and other cleanups
 */	
void loader_cleanup() {
  //checking if file header , program header , file pointer , virtual memory  has allocated memory and if they does , deallocating it
  if(ehdr!=NULL){
    free(ehdr);
  }
  if(phdr!=NULL){
    free(phdr);
   }
  
  if(file!=NULL){
    fclose(file);
   }
  if(virtual_mem){
	  munmap(virtual_mem,phdr[i].p_memsz);
   }
}
void load_ehdr_and_check(FILE * file){//loading file header and checking if file is ELF
	fseek(file,0,SEEK_SET);//sets pointer to start of file
	ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));//allocating memory
	if(!ehdr){//if failed to allocate memory returns error
	  printf("Error: Unable to allocate memory for Elf_hdr");
	  fclose(file);
	  exit(1);
	}
	if (fread(ehdr, sizeof(Elf32_Ehdr), 1, file) != 1) {//reading contious bytes of size of header into ehdr pointer
       	 printf("Error: Unable to load Elf_hdr");//if failed to read 
	 loader_cleanup();
       	 exit(1);
	}
	switch (ehdr->e_ident[EI_CLASS]) {//checking if file is ELF
   	   case ELFCLASS32:
        	// 32-bit ELF file
       	 	break;
    	   case ELFCLASS64:
        	printf("Error: Not a 32-bit ELF file\n");//given file is 64 bits 
		loader_cleanup();
       	 	exit(1);
    	   default:
        	printf("Error: Unsupported ELF file class\n");// given file is unsupported
		loader_cleanup();
        	exit(1);
	}
	if(ehdr->e_type!=ET_EXEC){//checking if given ELF file is executable type file
		printf("Error: ELF file in not executable");
		loader_cleanup();
		exit(1);
	}
	fseek(file,0,SEEK_SET);//reseting pointer to start of the file

	return;
}
void load_phdr_and_check(FILE *file){//loading program headers and checking if it is loading correctly
   fseek(file,0,SEEK_SET);//setting pointer to start of the file 

     phdr = (Elf32_Phdr*)malloc((ehdr -> e_phnum)*sizeof(Elf32_Phdr));//allocating memory for program headers = number of header * size of one program header
        if(!phdr){//if failed to allocate memory for program header
          printf("Error: Unable to allocate memory for Elf_Phdr");
	  loader_cleanup();
          exit(1);
        }
     fseek (file, ehdr->e_phoff, SEEK_SET);//offsetting file pointer by program header offset given in elf file header 
     if (fread(phdr, ehdr->e_phentsize, ehdr->e_phnum, file) != ehdr->e_phnum) {//reading continous bytes of memory into phdr which is number of program header * size of one program header
        printf("Error: Unable to load ELf_Phdr");//if failed to read  program header
	loader_cleanup();
       	exit(1);
    }
     return;
}

//function to check if given file is ELF executable
bool check_file(FILE *file){
	if(!file){//if file pointer is NULL 
	 	return 0;
	}
	load_ehdr_and_check(file);//loading file header and checking if given file is ELF executable
	return 1;
}
		



/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char * exe) {
  // 1. Load entire binary content into the memory from the ELF file.
  load_phdr_and_check(file);
  // 2. Iterate through the PHDR table and find the section of PT_LOAD 
  //    type that contains the address of the entrypoint method in fib.c
  fseek(file,0,SEEK_SET);
  Elf32_Addr entry_pt = 0 ;
  int min = 0xFFFFFFFF;
  i = 0  ;
  minentry = -1;
  int R_E[2] = {0x6,0x5};
  for ( i = 0; i < ehdr -> e_phnum ; i++)
  {
	  if ( phdr[i].p_flags == R_E[0] || phdr[i].p_flags == R_E[1] )
    {
      if (min > ehdr->e_entry - phdr[i].p_vaddr )
      {
        min = ehdr->e_entry - phdr[i].p_vaddr;
        minentry = i;
      }
    }
  }
  i = minentry;
  entry_pt = phdr[i].p_vaddr;


    if (minentry == -1) {//if no entry point is found during the for loop then no executable segment is there
        printf("Error: No suitable executable segment found.\n");
        loader_cleanup();
	exit(1);
    }

  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content
   virtual_mem = mmap(NULL, phdr[minentry].p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE , 0, 0);//allocating memory of size "p_memsz" into virtual memory
   if (virtual_mem == MAP_FAILED) {//if failed to allocate memory to virtual memory
      printf("Error: Unable to allocate virtual memory\n");
      loader_cleanup();
      exit(1);
  }
 
  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  fseek(file,0,SEEK_SET);//setting file pointer to starting of the file 
  fseek(file, phdr[i].p_offset, SEEK_SET);//setting file pointer to entrypoint address
  size_t bytes_read = fread(virtual_mem, 1, phdr[minentry].p_memsz, file);//reading continous bytes of sie "p_memsz" to virtual memory starting from entry point address
  if (bytes_read != phdr[minentry].p_memsz) {//if fail  to read sufficient amount of bytes 
        printf("Error: Unable to read the entire file");
        loader_cleanup();
        fclose(file);
        exit(1);
    }
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  Elf32_Addr offset = ehdr->e_entry - entry_pt;//calculating offset in between  entry point and segment starting address
  void *entry_virtual = virtual_mem + offset; // getting memory address for _start function
   int (*_start)() = (int(*)())entry_virtual;

  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("User _start return value = %d\n",result);
}

int main(int argc, char** argv) 
{	 
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  file = fopen(argv[1],"rb");
  // 1. carry out necessary checks on the input ELF file
  if(!check_file(file)){
	  printf("Error: Unable to open file");
	  fclose(file);
	  exit(1);
  }

  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv[1]);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}

