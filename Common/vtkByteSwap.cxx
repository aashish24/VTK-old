/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
#include "vtkByteSwap.h"
#include <memory.h>
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkByteSwap* vtkByteSwap::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkByteSwap");
  if(ret)
    {
    return (vtkByteSwap*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkByteSwap;
}




// Swap four byte word.
#ifdef VTK_WORDS_BIGENDIAN
void vtkByteSwap::Swap4BE(char *){}
#else
void vtkByteSwap::Swap4BE(char *mem_ptr1)
{
  char one_byte;

  one_byte    = mem_ptr1[0];
  mem_ptr1[0] = mem_ptr1[3];
  mem_ptr1[3] = one_byte;

  one_byte    = mem_ptr1[1];
  mem_ptr1[1] = mem_ptr1[2];
  mem_ptr1[2] = one_byte;
}
#endif

// Swap bunch of bytes. Num is the number of four byte words to swap.
#ifdef VTK_WORDS_BIGENDIAN
void vtkByteSwap::Swap4BERange(char *,int){}
#else
void vtkByteSwap::Swap4BERange(char *mem_ptr1,int num)
{
  char one_byte;
  char *pos;
  int i;
  
  pos = mem_ptr1;
  
  for (i = 0; i < num; i++)
    {
    one_byte = pos[0];
    pos[0] = pos[3];
    pos[3] = one_byte;
    
    one_byte = pos[1];
    pos[1] = pos[2];
    pos[2] = one_byte;
    pos = pos + 4;
    }
  
}
#endif



// Swap bunch of bytes. Num is the number of four byte words to swap.
void vtkByteSwap::SwapWrite4BERange(char *mem_ptr1,int num, ostream *fp)
{
#ifndef VTK_WORDS_BIGENDIAN
  char one_byte;
  char *pos;
  int i;
  char *cpy;
  
  cpy = new char [num*4];
  memcpy(cpy, mem_ptr1,num*4);
  
  pos = cpy;
  
  for (i = 0; i < num; i++)
    {
    one_byte = pos[0];
    pos[0] = pos[3];
    pos[3] = one_byte;
    
    one_byte = pos[1];
    pos[1] = pos[2];
    pos[2] = one_byte;
    pos = pos + 4;
    }
  fp->write((char *)cpy, 4*num);
  delete [] cpy;
  
#else
  fp->write((char *)mem_ptr1, 4*num);
#endif
}

// Swap bunch of bytes. Num is the number of four byte words to swap.
void vtkByteSwap::SwapWrite4BERange(char *mem_ptr1,int num, FILE *fp)
{
#ifndef VTK_WORDS_BIGENDIAN
  char one_byte;
  char *pos;
  int i;
  char *cpy;
  
  cpy = new char [num*4];
  memcpy(cpy, mem_ptr1,num*4);
  
  pos = cpy;
  
  for (i = 0; i < num; i++)
    {
    one_byte = pos[0];
    pos[0] = pos[3];
    pos[3] = one_byte;
    
    one_byte = pos[1];
    pos[1] = pos[2];
    pos[2] = one_byte;
    pos = pos + 4;
    }
  fwrite(cpy,4,num,fp);
  delete [] cpy;
  
#else
  fwrite(mem_ptr1,4,num,fp);
#endif
}

// Swap 2 byte word.
#ifdef VTK_WORDS_BIGENDIAN
void vtkByteSwap::Swap2LE(short *mem_ptr)
{
  unsigned short h1,h2;

  h1 = (unsigned short) *mem_ptr << 8;
  h2 = (unsigned short) *mem_ptr >> 8;
  *mem_ptr = (short) h1 | h2;

}
#else
void vtkByteSwap::Swap2LE(short *) {}
#endif

// Swap four byte word.
#ifdef VTK_WORDS_BIGENDIAN
void vtkByteSwap::Swap4LE(char *mem_ptr1)
{
  char one_byte;

  one_byte    = mem_ptr1[0];
  mem_ptr1[0] = mem_ptr1[3];
  mem_ptr1[3] = one_byte;

  one_byte    = mem_ptr1[1];
  mem_ptr1[1] = mem_ptr1[2];
  mem_ptr1[2] = one_byte;
}
#else
void vtkByteSwap::Swap4LE(char *){}
#endif

// Swap bunch of bytes. Num is the number of four byte words to swap.
#ifdef VTK_WORDS_BIGENDIAN
void vtkByteSwap::Swap4LERange(char *mem_ptr1,int num)
{
  char one_byte;
  char *pos;
  int i;
  
  pos = mem_ptr1;
  
  for (i = 0; i < num; i++)
    {
    one_byte    = pos[0];
    pos[0] = pos[3];
    pos[3] = one_byte;
    
    one_byte    = pos[1];
    pos[1] = pos[2];
    pos[2] = one_byte;
    pos = pos + 4;
    }
  
}
#else
void vtkByteSwap::Swap4LERange(char *,int) {}
#endif

// Swap 2 byte word.
#ifdef VTK_WORDS_BIGENDIAN
void vtkByteSwap::Swap2BE(short *) {}
#else
void vtkByteSwap::Swap2BE(short *mem_ptr)
{
  unsigned short h1,h2;

  h1 = (unsigned short)*mem_ptr << 8;
  h2 = (unsigned short)*mem_ptr >> 8;
  *mem_ptr = (short) h1 | h2;

}
#endif

// Swap bunch of bytes. Num is the number of two byte words to swap.
#ifdef VTK_WORDS_BIGENDIAN
void vtkByteSwap::Swap2BERange(char *,int) {}
#else
void vtkByteSwap::Swap2BERange(char *mem_ptr1,int num)
{
  char one_byte;
  char *pos;
  int i;
  
  pos = mem_ptr1;
  
  for (i = 0; i < num; i++)
    {
    one_byte = pos[0];
    pos[0] = pos[1];
    pos[1] = one_byte;
    pos = pos + 2;
    }
  
}
#endif

// Swap bunch of bytes. Num is the number of two byte words to swap.
#ifdef VTK_WORDS_BIGENDIAN
void vtkByteSwap::Swap2LERange(char *mem_ptr1,int num)
{
  char one_byte;
  char *pos;
  int i;
  
  pos = mem_ptr1;
  
  for (i = 0; i < num; i++)
    {
    one_byte = pos[0];
    pos[0] = pos[1];
    pos[1] = one_byte;
    pos = pos + 2;
    }
  
}
#else
void vtkByteSwap::Swap2LERange(char *mem_ptr1,int num){}
#endif



// Swap bunch of bytes. Num is the number of four byte words to swap.
void vtkByteSwap::SwapWrite2BERange(char *mem_ptr1,int num, ostream *fp)
{
#ifndef VTK_WORDS_BIGENDIAN
  char one_byte;
  char *pos;
  int i;
  char *cpy;
  
  cpy = new char [num*2];
  memcpy(cpy, mem_ptr1,num*2);
  
  pos = cpy;
  
  for (i = 0; i < num; i++)
    {
    one_byte = pos[0];
    pos[0] = pos[1];
    pos[1] = one_byte;
    pos = pos + 2;
    }
  fp->write((char *)cpy, 2*num);

  delete [] cpy;
  
#else
  fp->write((char *)mem_ptr1, 2*num);
#endif
}

// Swap bunch of bytes. Num is the number of four byte words to swap.
void vtkByteSwap::SwapWrite2BERange(char *mem_ptr1,int num, FILE *fp)
{
#ifndef VTK_WORDS_BIGENDIAN
  char one_byte;
  char *pos;
  int i;
  char *cpy;
  
  cpy = new char [num*2];
  memcpy(cpy, mem_ptr1,num*2);
  
  pos = cpy;
  
  for (i = 0; i < num; i++)
    {
    one_byte = pos[0];
    pos[0] = pos[1];
    pos[1] = one_byte;
    pos = pos + 2;
    }
  fwrite(cpy,2,num,fp);
  delete [] cpy;
  
#else
  fwrite(mem_ptr1,2,num,fp);
#endif
}

//----------------------------------------------------------------------------
// Swaps the bytes of a buffer.  Uses an arbitrary word size, but
// assumes the word size is divisible by two.
void vtkByteSwap::SwapVoidRange(void *buffer, int numWords, int wordSize)
{
  unsigned char temp, *out, *buf;
  int idx1, idx2, inc, half;
  
  half = wordSize / 2;
  inc = wordSize - 1;
  buf = (unsigned char *)(buffer);
  
  for (idx1 = 0; idx1 < numWords; ++idx1)
    {
    out = buf + inc;
    for (idx2 = 0; idx2 < half; ++idx2)
      {
      temp = *out;
      *out = *buf;
      *buf = temp;
      ++buf;
      --out;
      }
    buf += half;
    }
}

  
    
