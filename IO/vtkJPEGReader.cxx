/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkJPEGReader.h"
#include "vtkObjectFactory.h"

extern "C" {
#include <jpeglib.h>
#include <setjmp.h>
}


vtkCxxRevisionMacro(vtkJPEGReader, "$Revision$");
vtkStandardNewMacro(vtkJPEGReader);


// create an error handler for jpeg that
// can longjmp out of the jpeg library 
struct vtk_jpeg_error_mgr 
{
  struct jpeg_error_mgr pub;    /* "public" fields */
  jmp_buf setjmp_buffer;        /* for return to caller */
  vtkJPEGReader* JPEGReader;
};

// this is called on jpeg error conditions
extern "C" void vtk_jpeg_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  vtk_jpeg_error_mgr * err = reinterpret_cast<vtk_jpeg_error_mgr*>(cinfo->err);

  /* Return control to the setjmp point */
  longjmp(err->setjmp_buffer, 1);
}

extern "C" void vtk_jpeg_output_message (j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];

  /* Create the message */
  (*cinfo->err->format_message) (cinfo, buffer);
  vtk_jpeg_error_mgr * err = reinterpret_cast<vtk_jpeg_error_mgr*>(cinfo->err);
  vtkWarningWithObjectMacro(err->JPEGReader,
                            "libjpeg error: " <<  buffer);
}


void vtkJPEGReader::ExecuteInformation()
{
  this->ComputeInternalFileName(this->DataExtent[4]);
  if (this->InternalFileName == NULL)
    {
    return;
    }

  FILE *fp = fopen(this->InternalFileName, "rb");
  if (!fp)
    {
    vtkErrorWithObjectMacro(this, 
                            "Unable to open file " 
                            << this->InternalFileName);
    return;
    }

  // create jpeg decompression object and error handler
  struct jpeg_decompress_struct cinfo;
  struct vtk_jpeg_error_mgr jerr;
  jerr.JPEGReader = this;

  cinfo.err = jpeg_std_error(&jerr.pub); 
  // for any jpeg error call vtk_jpeg_error_exit
  jerr.pub.error_exit = vtk_jpeg_error_exit;
  // for any output message call vtk_jpeg_output_message
  jerr.pub.output_message = vtk_jpeg_output_message;
  if (setjmp(jerr.setjmp_buffer))
    {
    // clean up
    jpeg_destroy_decompress(&cinfo);
    // close the file
    fclose(fp);
    // this is not a valid jpeg file 
    vtkErrorWithObjectMacro(this, "libjpeg could not read file: "
                            << this->InternalFileName);
    return;
    }
  jpeg_create_decompress(&cinfo);

  // set the source file
  jpeg_stdio_src(&cinfo, fp);

  // read the header
  jpeg_read_header(&cinfo, TRUE);

  // force the output image size to be calculated (we could have used
  // cinfo.image_height etc. but that would preclude using libjpeg's
  // ability to scale an image on input).
  jpeg_calc_output_dimensions(&cinfo);

  // pull out the width/height, etc.
  this->DataExtent[0] = 0;
  this->DataExtent[1] = cinfo.output_width - 1;
  this->DataExtent[2] = 0;
  this->DataExtent[3] = cinfo.output_height - 1;

  this->SetDataScalarTypeToUnsignedChar();
  this->SetNumberOfScalarComponents( cinfo.output_components );

  this->vtkImageReader2::ExecuteInformation();

  // close the file
  jpeg_destroy_decompress(&cinfo);
  fclose(fp);
}


template <class OT>
static void vtkJPEGReaderUpdate2(vtkJPEGReader *self, OT *outPtr,
                                int *outExt, int *outInc, long)
{
  unsigned int ui;
  int i;
  FILE *fp = fopen(self->GetInternalFileName(), "rb");
  if (!fp)
    {
    return;
    }

  // create jpeg decompression object and error handler
  struct jpeg_decompress_struct cinfo;
  struct vtk_jpeg_error_mgr jerr;
  jerr.JPEGReader = self;

  cinfo.err = jpeg_std_error(&jerr.pub);
  // for any jpeg error call vtk_jpeg_error_exit
  jerr.pub.error_exit = vtk_jpeg_error_exit;
  // for any output message call vtk_jpeg_output_message
  jerr.pub.output_message = vtk_jpeg_output_message;
  if (setjmp(jerr.setjmp_buffer))
    {
    // clean up
    jpeg_destroy_decompress(&cinfo);
    // close the file
    fclose(fp);
    vtkErrorWithObjectMacro(self, "libjpeg could not read file: "
                            << self->GetInternalFileName());
    // this is not a valid jpeg file
    return;
    }
  jpeg_create_decompress(&cinfo);

  // set the source file
  jpeg_stdio_src(&cinfo, fp);

  // read the header
  jpeg_read_header(&cinfo, TRUE);

  // prepare to read the bulk data
  jpeg_start_decompress(&cinfo);

  
  int rowbytes = cinfo.output_components * cinfo.output_width;
  unsigned char *tempImage = new unsigned char [rowbytes*cinfo.output_height];
  JSAMPROW *row_pointers = new JSAMPROW [cinfo.output_height];
  for (ui = 0; ui < cinfo.output_height; ++ui)
    {
    row_pointers[ui] = tempImage + rowbytes*ui;
    }

  // read the bulk data
  unsigned int remainingRows = cinfo.output_height;
  while (cinfo.output_scanline < cinfo.output_height)
    {
    remainingRows = cinfo.output_height - cinfo.output_scanline;
    jpeg_read_scanlines(&cinfo, &row_pointers[cinfo.output_scanline],
                        remainingRows);
    }

  // finish the decompression step
  jpeg_finish_decompress(&cinfo);

  // destroy the decompression object
  jpeg_destroy_decompress(&cinfo);

  // copy the data into the outPtr
  OT *outPtr2;
  outPtr2 = outPtr;
  long outSize = cinfo.output_components*(outExt[1] - outExt[0] + 1);
  for (i = outExt[2]; i <= outExt[3]; ++i)
    {
    memcpy(outPtr2,
           row_pointers[cinfo.output_height - i - 1]
           + outExt[0]*cinfo.output_components,
           outSize);
    outPtr2 += outInc[1];
    }
  delete [] tempImage;
  delete [] row_pointers;

  // close the file
  fclose(fp);
}

//----------------------------------------------------------------------------
// This function reads in one data of data.
// templated to handle different data types.
template <class OT>
static void vtkJPEGReaderUpdate(vtkJPEGReader *self, vtkImageData *data, 
                               OT *outPtr)
{
  int outIncr[3];
  int outExtent[6];
  OT *outPtr2;

  data->GetExtent(outExtent);
  data->GetIncrements(outIncr);

  long pixSize = data->GetNumberOfScalarComponents()*sizeof(OT);  
  
  outPtr2 = outPtr;
  int idx2;
  for (idx2 = outExtent[4]; idx2 <= outExtent[5]; ++idx2)
    {
    self->ComputeInternalFileName(idx2);
    // read in a JPEG file
    vtkJPEGReaderUpdate2(self, outPtr2, outExtent, outIncr, pixSize);
    self->UpdateProgress((idx2 - outExtent[4])/
                         (outExtent[5] - outExtent[4] + 1.0));
    outPtr2 += outIncr[2];
    }
}


//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
void vtkJPEGReader::ExecuteData(vtkDataObject *output)
{
  vtkImageData *data = this->AllocateOutputData(output);

  if (this->InternalFileName == NULL)
    {
    vtkErrorMacro(<< "Either a FileName or FilePrefix must be specified.");
    return;
    }

  this->ComputeDataIncrements();
  
  // Call the correct templated function for the output
  void *outPtr;

  // Call the correct templated function for the input
  outPtr = data->GetScalarPointer();
  switch (data->GetScalarType())
    {
    vtkTemplateMacro3(vtkJPEGReaderUpdate, this, data, (VTK_TT *)(outPtr));
    default:
      vtkErrorMacro(<< "UpdateFromFile: Unknown data type");
    }   
}



int vtkJPEGReader::CanReadFile(const char* fname)
{
  // open the file
  FILE *fp = fopen(fname, "rb");
  if (!fp)
    {
    return 0;
    }
  // read the first two bytes
  char magic[2];
  int n = fread(magic, sizeof(magic), 1, fp);
  if (n != 1) 
    {
    fclose(fp);
    return 0;
    }
  // check for the magic stuff:
  // 0xFF followed by 0xD8
  if( ( (magic[0] != char(0xFF)) || (magic[1] != char(0xD8)) ))
    {
    fclose(fp);
    return 0;
    }
  // go back to the start of the file
  fseek(fp, 0, SEEK_SET);
  // magic number is ok, try and read the header
  struct vtk_jpeg_error_mgr jerr;
  jerr.JPEGReader = this;
  struct jpeg_decompress_struct cinfo;
  cinfo.err = jpeg_std_error(&jerr.pub);
  // for any jpeg error call vtk_jpeg_error_exit
  jerr.pub.error_exit = vtk_jpeg_error_exit;
  // for any output message call vtk_jpeg_error_exit
  jerr.pub.output_message = vtk_jpeg_error_exit;
  // set the jump point, if there is a jpeg error or warning
  // this will evaluate to true
  if (setjmp(jerr.setjmp_buffer))
    {
    // clean up
    jpeg_destroy_decompress(&cinfo);
    // close the file
    fclose(fp);
    // this is not a valid jpeg file
    return 0;
    }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);
  /* Step 2: specify data source (eg, a file) */
  jpeg_stdio_src(&cinfo, fp);
  /* Step 3: read file parameters with jpeg_read_header() */
  jpeg_read_header(&cinfo, TRUE);
  
  // if no errors have occurred yet, then it must be jpeg
  jpeg_destroy_decompress(&cinfo);
  fclose(fp);
  return 1;
}
