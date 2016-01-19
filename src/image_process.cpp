#include "image_process.h"
#include "FreeImage.h"
#include "log.h"

#ifdef UNICODE
#define FreeImage_Load	FreeImage_LoadU
#define FreeImage_Save	FreeImage_SaveU
#define FreeImage_GetFIFFromFilename FreeImage_GetFIFFromFilenameU
#define FreeImage_GetFileWH			 FreeImage_GetFileWHU
#endif

LOG_V_MODULE(log_cbv, _T("image_process"), g_global_log_output, 0);


static void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) 
{
	const char* fmt = NULL;
	if(fif != FIF_UNKNOWN) {
		fmt = FreeImage_GetFormatFromFIF(fif);
		LOG_CB_V(log_cbv, LOG_MSG, _T("/*** %s : %s ***/"), fmt, message);
	}
	else {
		LOG_CB_V(log_cbv, LOG_MSG, _T("/*** FIF_UNKNOWN : %s ***/"), message);
	}
}

void image_debug_enable(int enable)
{
	if (enable)
		 FreeImage_SetOutputMessage(FreeImageErrorHandler);
	else
		FreeImage_SetOutputMessage(NULL);
}

int image_size_get(LPCTSTR file, int& width, int& height)
{
	FREE_IMAGE_FORMAT type;
	FIBITMAP* bitmap;
	BOOL ok;

	type = FreeImage_GetFIFFromFilename(file);
	if (FIF_UNKNOWN == type){
		LOG_CB_V(log_cbv, LOG_ERR, _T("image type unknow %s"), file);
		return -1;
	}

	ok = FreeImage_GetFileWH((unsigned*)&width, (unsigned*)&height, type, file, 0);
	if (!ok){
		LOG_CB_V(log_cbv, LOG_DEBUG, _T("FreeImage_GetFileWH %s fail, use FreeImage_Load"), file);
		
		bitmap = FreeImage_Load(type, file, FIF_LOAD_NOPIXELS);
		if (NULL == -1){
			LOG_CB_V(log_cbv, LOG_ERR, _T("FreeImage_Load %s fail"), file);
			return -1;
		}

		width = FreeImage_GetWidth(bitmap);
		height = FreeImage_GetHeight(bitmap);

		FreeImage_Unload(bitmap);
	}

	return 0;
}

int image_clipping(LPCTSTR dest, LPCTSTR src, int left, int top, int right, int bottom, int width, int height, int filter)
{
	int  rc = 0;
	FREE_IMAGE_FORMAT type;
	FIBITMAP* bitmapfrom, *bitmapto;
	
	type = FreeImage_GetFIFFromFilename(src);
	if (FIF_UNKNOWN == type){
		LOG_CB_V(log_cbv, LOG_ERR, _T("image type unknow %s"), src);
		return -1;
	}

	bitmapfrom = FreeImage_Load(type, src, 0);
	if (NULL == bitmapfrom){
		LOG_CB_V(log_cbv, LOG_ERR, _T("FreeImage_Load %s fail"), src);
		return -1;
	}

	bitmapto = FreeImage_RescaleRect(bitmapfrom, width, height, left, top, right, bottom, (FREE_IMAGE_FILTER)filter, 0);
	if (NULL == bitmapfrom){
		LOG_CB_V(log_cbv, LOG_ERR, _T("FreeImage_RescaleRect type %d fail"), filter);
		goto err;
	}

	if(!FreeImage_Save(type, bitmapto, dest, 0)){
		LOG_CB_V(log_cbv, LOG_ERR, _T("FreeImage_Save %s fail"), dest);
		goto err;
	}

	FreeImage_Unload(bitmapfrom);
	FreeImage_Unload(bitmapto);
	return 0;
	
err:
	if (bitmapfrom)
		FreeImage_Unload(bitmapfrom);
	if (bitmapto)
		FreeImage_Unload(bitmapto);
	return -1;
}

