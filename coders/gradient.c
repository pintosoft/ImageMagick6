/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%            GGGG  RRRR    AAA   DDDD   IIIII  EEEEE  N   N  TTTTT            %
%           G      R   R  A   A  D   D    I    E      NN  N    T              %
%           G  GG  RRRR   AAAAA  D   D    I    EEE    N N N    T              %
%           G   G  R R    A   A  D   D    I    E      N  NN    T              %
%            GGG   R  R   A   A  DDDD   IIIII  EEEEE  N   N    T              %
%                                                                             %
%                                                                             %
%                   Read An Image Filled Using Gradient.                      %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
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
#include "magick/channel.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace-private.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/paint.h"
#include "magick/pixel-accessor.h"
#include "magick/pixel-private.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/studio.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d G R A D I E N T I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadGRADIENTImage creates a gradient image and initializes it to
%  the color range as specified by the filename.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  The format of the ReadGRADIENTImage method is:
%
%      Image *ReadGRADIENTImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static Image *ReadXCImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  IndexPacket
    index,
    *indexes;

  MagickBooleanType
    status;

  MagickPixelPacket
    color;

  PixelPacket
    pixel;

  ssize_t
    x;

  PixelPacket
    *q;

  ssize_t
    y;

  /*
    Initialize Image structure.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image=AcquireImage(image_info);
  if (image->columns == 0)
    image->columns=1;
  if (image->rows == 0)
    image->rows=1;
  status=SetImageExtent(image,image->columns,image->rows);
  if (status == MagickFalse)
    {
      InheritException(exception,&image->exception);
      return(DestroyImageList(image));
    }
  (void) CopyMagickString(image->filename,image_info->filename,MaxTextExtent);
  status=QueryMagickColor((char *) image_info->filename,&color,exception);
  if (status == MagickFalse)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  (void) SetImageColorspace(image,color.colorspace);
  image->matte=color.matte;
  (void) memset(&pixel,0,sizeof(pixel));
  index=0;
  SetPixelPacket(image,&color,&pixel,&index);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
      *q++=pixel;
    if (image->colorspace == CMYKColorspace)
      {
        indexes=GetAuthenticIndexQueue(image);
        for (x=0; x < (ssize_t) image->columns; x++)
          SetPixelIndex(indexes+x,index);
      }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
  }
  return(GetFirstImageInList(image));
}

static Image *ReadGRADIENTImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    colorname[MaxTextExtent+4];

  Image
    *image;

  ImageInfo
    *read_info;

  MagickBooleanType
    icc_color,
    status;

  MagickPixelPacket
    start_pixel,
    stop_pixel;

  PixelPacket
    start_color,
    stop_color;

  /*
    Initialize Image structure.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) CopyMagickString(colorname,image_info->filename,MaxTextExtent);
  (void) sscanf(image_info->filename,"%[^-]",colorname);
  (void) CopyMagickString(read_info->filename,colorname,MaxTextExtent);
  image=ReadXCImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  (void) SetImageOpacity(image,(Quantum) TransparentOpacity);
  (void) CopyMagickString(image->filename,image_info->filename,MaxTextExtent);
  icc_color=MagickFalse;
  if (LocaleCompare(colorname,"icc") == 0)
    {
      (void) ConcatenateMagickString(colorname,"-",MaxTextExtent);
      (void) sscanf(image_info->filename,"%*[^-]-%[^-]",colorname+4);
      icc_color=MagickTrue;
    }
  if (QueryColorDatabase(colorname,&start_color,exception) == MagickFalse)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  (void) QueryMagickColor(colorname,&start_pixel,exception);
  (void) CopyMagickString(colorname,"white",MaxTextExtent);
  if (GetPixelLuma(image,&start_color) > (QuantumRange/2.0))
    (void) CopyMagickString(colorname,"black",MaxTextExtent);
  if (icc_color == MagickFalse)
    (void) sscanf(image_info->filename,"%*[^-]-%[^-]",colorname);
  else
    (void) sscanf(image_info->filename,"%*[^-]-%*[^-]-%[^-]",colorname);
  if (QueryColorDatabase(colorname,&stop_color,exception) == MagickFalse)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  (void) QueryMagickColor(colorname,&stop_pixel,exception);
  (void) SetImageColorspace(image,start_pixel.colorspace);
  image->matte=start_pixel.matte;
  if (stop_pixel.matte != MagickFalse)
    image->matte=MagickTrue;
  status=GradientImage(image,LocaleCompare(image_info->magick,"GRADIENT") == 0 ?
    LinearGradient : RadialGradient,PadSpread,&start_color,&stop_color);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r G R A D I E N T I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterGRADIENTImage() adds attributes for the GRADIENT image format
%  to the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterGRADIENTImage method is:
%
%      size_t RegisterGRADIENTImage(void)
%
*/
ModuleExport size_t RegisterGRADIENTImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("GRADIENT");
  entry->decoder=(DecodeImageHandler *) ReadGRADIENTImage;
  entry->adjoin=MagickFalse;
  entry->raw=MagickTrue;
  entry->format_type=ImplicitFormatType;
  entry->description=ConstantString("Gradual linear passing from one shade to "
    "another");
  entry->magick_module=ConstantString("GRADIENT");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("RADIAL-GRADIENT");
  entry->decoder=(DecodeImageHandler *) ReadGRADIENTImage;
  entry->adjoin=MagickFalse;
  entry->raw=MagickTrue;
  entry->format_type=ImplicitFormatType;
  entry->description=ConstantString("Gradual radial passing from one shade to "
    "another");
  entry->magick_module=ConstantString("GRADIENT");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r G R A D I E N T I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterGRADIENTImage() removes format registrations made by the
%  GRADIENT module from the list of supported formats.
%
%  The format of the UnregisterGRADIENTImage method is:
%
%      UnregisterGRADIENTImage(void)
%
*/
ModuleExport void UnregisterGRADIENTImage(void)
{
  (void) UnregisterMagickInfo("RADIAL-GRADIENT");
  (void) UnregisterMagickInfo("GRADIENT");
}
