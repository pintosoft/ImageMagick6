/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     V   V  IIIII  DDDD   EEEEE   OOO                        %
%                     V   V    I    D   D  E      O   O                       %
%                     V   V    I    D   D  EEE    O   O                       %
%                      V V     I    D   D  E      O   O                       %
%                       V    IIIII  DDDD   EEEEE   OOO                        %
%                                                                             %
%                                                                             %
%                       Read/Write VIDEO Image Format                         %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1999                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999 ImageMagick Studio LLC, a non-profit organization           %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/
/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/constitute.h"
#include "magick/delegate.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/layer.h"
#include "magick/list.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/transform.h"
#include "magick/utility.h"
#include "magick/utility-private.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteVIDEOImage(const ImageInfo *image_info,Image *image);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s V I D E O                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsVIDEO() returns MagickTrue if the image format type, identified by the
%  magick string, is VIDEO.
%
%  The format of the IsVIDEO method is:
%
%      MagickBooleanType IsVIDEO(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/

static MagickBooleanType IsPNG(const unsigned char *magick,const size_t length)
{
  if (length < 8)
    return(MagickFalse);
  if (memcmp(magick,"\211PNG\r\n\032\n",8) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

static MagickBooleanType IsVIDEO(const unsigned char *magick,
  const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\000\000\001\263",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d V I D E O I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadVIDEOImage() reads an binary file in the VIDEO video stream format
%  and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadVIDEOImage method is:
%
%      Image *ReadVIDEOImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadVIDEOImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define ReadVIDEOIntermediateFormat "pam"

  Image
    *image,
    *images,
    *next;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  (void) CloseBlob(image);
  (void) DestroyImageList(image);
  /*
    Convert VIDEO to PAM with delegate.
  */
  images=(Image *) NULL;
  read_info=CloneImageInfo(image_info);
  image=AcquireImage(image_info);
  status=InvokeDelegate(read_info,image,"video:decode",(char *) NULL,exception);
  if (status != MagickFalse)
    {
      (void) FormatLocaleString(read_info->filename,MaxTextExtent,"%s.%s",
        read_info->unique,ReadVIDEOIntermediateFormat);
      *read_info->magick='\0';
      images=ReadImage(read_info,exception);
      if (images != (Image *) NULL)
        for (next=images; next != (Image *) NULL; next=next->next)
        {
          (void) CopyMagickString(next->filename,image->filename,
            MagickPathExtent);
          (void) CopyMagickString(next->magick,image->magick,MagickPathExtent);
        }
      (void) RelinquishUniqueFileResource(read_info->filename);
    }
  read_info=DestroyImageInfo(read_info);
  image=DestroyImage(image);
  return(images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r V I D E O I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterVIDEOImage() adds attributes for the VIDEO image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterVIDEOImage method is:
%
%      size_t RegisterVIDEOImage(void)
%
*/
ModuleExport size_t RegisterVIDEOImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("3GP");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Media Container");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("3G2");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Media Container");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("APNG");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsPNG;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Animated Portable Network Graphics");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("AVI");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Microsoft Audio/Visual Interleaved");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("MKV");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Multimedia Container");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("MOV");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("MPEG Video Stream");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("MPEG");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("MPEG Video Stream");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("MPG");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("MPEG Video Stream");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("MP4");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("VIDEO-4 Video Stream");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("M2V");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("MPEG Video Stream");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("M4V");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Raw VIDEO-4 Video");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("VIDEO");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("MPEG Video Stream");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("WEBM");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Open Web Media");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("WMV");
  entry->decoder=(DecodeImageHandler *) ReadVIDEOImage;
  entry->encoder=(EncodeImageHandler *) WriteVIDEOImage;
  entry->magick=(IsImageFormatHandler *) IsVIDEO;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("Windows Media Video");
  entry->magick_module=ConstantString("VIDEO");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r V I D E O I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterVIDEOImage() removes format registrations made by the
%  BIM module from the list of supported formats.
%
%  The format of the UnregisterBIMImage method is:
%
%      UnregisterVIDEOImage(void)
%
*/
ModuleExport void UnregisterVIDEOImage(void)
{
  (void) UnregisterMagickInfo("WMV");
  (void) UnregisterMagickInfo("WEBM");
  (void) UnregisterMagickInfo("MOV");
  (void) UnregisterMagickInfo("M4V");
  (void) UnregisterMagickInfo("M2V");
  (void) UnregisterMagickInfo("MP4");
  (void) UnregisterMagickInfo("MPG");
  (void) UnregisterMagickInfo("MPEG");
  (void) UnregisterMagickInfo("MKV");
  (void) UnregisterMagickInfo("AVI");
  (void) UnregisterMagickInfo("APNG");
  (void) UnregisterMagickInfo("3G2");
  (void) UnregisterMagickInfo("3GP");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e V I D E O I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteVIDEOImage() writes an image to a file in VIDEO video stream format.
%  Lawrence Livermore National Laboratory (LLNL) contributed code to adjust
%  the VIDEO parameters to correspond to the compression quality setting.
%
%  The format of the WriteVIDEOImage method is:
%
%      MagickBooleanType WriteVIDEOImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType CopyDelegateFile(const char *source,
  const char *destination)
{
  int
    destination_file,
    source_file;

  MagickBooleanType
    status;

  size_t
    i;

  size_t
    length,
    quantum;

  ssize_t
    count;

  struct stat
    attributes;

  unsigned char
    *buffer;

  /*
    Return if destination file already exists and is not empty.
  */
  assert(source != (const char *) NULL);
  assert(destination != (char *) NULL);
  status=GetPathAttributes(destination,&attributes);
  if ((status != MagickFalse) && (attributes.st_size > 0))
    return(MagickTrue);
  /*
    Copy source file to destination.
  */
  if (strcmp(destination,"-") == 0)
    destination_file=fileno(stdout);
  else
    destination_file=open_utf8(destination,O_WRONLY | O_BINARY | O_CREAT,
      S_MODE);
  if (destination_file == -1)
    return(MagickFalse);
  source_file=open_utf8(source,O_RDONLY | O_BINARY,0);
  if (source_file == -1)
    {
      (void) close(destination_file);
      return(MagickFalse);
    }
  quantum=(size_t) MagickMaxBufferExtent;
  if ((fstat(source_file,&attributes) == 0) && (attributes.st_size > 0))
    quantum=(size_t) MagickMin((double) attributes.st_size,
      MagickMaxBufferExtent);
  buffer=(unsigned char *) AcquireQuantumMemory(quantum,sizeof(*buffer));
  if (buffer == (unsigned char *) NULL)
    {
      (void) close(source_file);
      (void) close(destination_file);
      return(MagickFalse);
    }
  length=0;
  for (i=0; ; i+=count)
  {
    count=(ssize_t) read(source_file,buffer,quantum);
    if (count <= 0)
      break;
    length=(size_t) count;
    count=(ssize_t) write(destination_file,buffer,length);
    if ((size_t) count != length)
      break;
  }
  if (strcmp(destination,"-") != 0)
    (void) close(destination_file);
  (void) close(source_file);
  buffer=(unsigned char *) RelinquishMagickMemory(buffer);
  return(i != 0 ? MagickTrue : MagickFalse);
}

static MagickBooleanType WriteVIDEOImage(const ImageInfo *image_info,
  Image *image)
{
#define WriteVIDEOIntermediateFormat "pam"

  char
    basename[MaxTextExtent],
    filename[MaxTextExtent];

  double
    delay;

  Image
    *clone_image;

  ImageInfo
    *write_info;

  int
    file;

  MagickBooleanType
    status;

  Image
    *p;

  ssize_t
    i;

  size_t
    count,
    length,
    scene;

  unsigned char
    *blob;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  (void) CloseBlob(image);
  /*
    Write intermediate files.
  */
  clone_image=CloneImageList(image,&image->exception);
  if (clone_image == (Image *) NULL)
    return(MagickFalse);
  file=AcquireUniqueFileResource(basename);
  if (file != -1)
    file=close(file)-1;
  (void) FormatLocaleString(clone_image->filename,MaxTextExtent,"%s",
    basename);
  count=0;
  write_info=CloneImageInfo(image_info);
  *write_info->magick='\0';
  for (p=clone_image; p != (Image *) NULL; p=GetNextImageInList(p))
  {
    char
      previous_image[MaxTextExtent];

    blob=(unsigned char *) NULL;
    length=0;
    scene=p->scene;
    delay=100.0*p->delay/MagickMax(1.0*p->ticks_per_second,1.0);
    for (i=0; i < (ssize_t) MagickMax((1.0*delay+1.0)/3.0,1.0); i++)
    {
      p->scene=count;
      count++;
      status=MagickFalse;
      switch (i)
      {
        case 0:
        {
          Image
            *frame;

          (void) FormatLocaleString(p->filename,MaxTextExtent,"%s%.20g.%s",
            basename,(double) p->scene,WriteVIDEOIntermediateFormat);
          (void) FormatLocaleString(filename,MaxTextExtent,"%s%.20g.%s",
            basename,(double) p->scene,WriteVIDEOIntermediateFormat);
          (void) FormatLocaleString(previous_image,MaxTextExtent,
            "%s%.20g.%s",basename,(double) p->scene,
            WriteVIDEOIntermediateFormat);
          frame=CloneImage(p,0,0,MagickTrue,&p->exception);
          if (frame == (Image *) NULL)
            break;
          status=WriteImage(write_info,frame);
          frame=DestroyImage(frame);
          break;
        }
        case 1:
        {
          blob=(unsigned char *) FileToBlob(previous_image,~0UL,&length,
            &image->exception);
          magick_fallthrough;
        }
        default:
        {
          (void) FormatLocaleString(filename,MaxTextExtent,"%s%.20g.%s",
            basename,(double) p->scene,WriteVIDEOIntermediateFormat);
          if (length > 0)
            status=BlobToFile(filename,blob,length,&image->exception);
          break;
        }
      }
      if (image->debug != MagickFalse)
        {
          if (status != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "%.20g. Wrote %s file for scene %.20g:",(double) i,
              WriteVIDEOIntermediateFormat,(double) p->scene);
          else
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "%.20g. Failed to write %s file for scene %.20g:",(double) i,
              WriteVIDEOIntermediateFormat,(double) p->scene);
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),"%s",filename);
        }
    }
    p->scene=scene;
    if (blob != (unsigned char *) NULL)
      blob=(unsigned char *) RelinquishMagickMemory(blob);
    if (status == MagickFalse)
      break;
  }
  /*
    Convert PAM to VIDEO.
  */
  (void) CopyMagickString(clone_image->magick_filename,basename,
    MaxTextExtent);
  (void) CopyMagickString(clone_image->filename,basename,MaxTextExtent);
  (void) CopyMagickString(clone_image->magick,image_info->magick,
    MaxTextExtent);
  status=InvokeDelegate(write_info,clone_image,(char *) NULL,"video:encode",
    &image->exception);
  (void) FormatLocaleString(write_info->filename,MaxTextExtent,"%s.%s",
    write_info->unique,clone_image->magick);
  status=CopyDelegateFile(write_info->filename,image->filename);
  (void) RelinquishUniqueFileResource(write_info->filename);
  write_info=DestroyImageInfo(write_info);
  /*
    Relinquish resources.
  */
  count=0;
  for (p=clone_image; p != (Image *) NULL; p=GetNextImageInList(p))
  {
    delay=100.0*p->delay/MagickMax(1.0*p->ticks_per_second,1.0);
    for (i=0; i < (ssize_t) MagickMax((1.0*delay+1.0)/3.0,1.0); i++)
    {
      (void) FormatLocaleString(p->filename,MaxTextExtent,"%s%.20g.%s",
        basename,(double) count++,WriteVIDEOIntermediateFormat);
      (void) RelinquishUniqueFileResource(p->filename);
    }
    (void) CopyMagickString(p->filename,image_info->filename,MaxTextExtent);
  }
  (void) RelinquishUniqueFileResource(basename);
  clone_image=DestroyImageList(clone_image);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"exit");
  return(status);
}
