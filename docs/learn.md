# data flow

when `MA_API ma_result ma_decoder_init_file`
is called, it first save a copy of pConfig, then call `ma_decoder__preinit_file`

```cpp
static ma_result ma_decoder__preinit_file(const char* pFilePath, const ma_decoder_config* pConfig, ma_decoder* pDecoder)
{
    ma_result result;

    result = ma_decoder__preinit(NULL, NULL, NULL, NULL, pConfig, pDecoder);
    if (result != MA_SUCCESS) {
        return result;
    }

    if (pFilePath == NULL || pFilePath[0] == '\0') {
        return MA_INVALID_ARGS;
    }

    return MA_SUCCESS;
}
```
then `ma_decoder__preinit` is called, 
```cpp
static ma_result ma_decoder__preinit(ma_decoder_read_proc onRead, ma_decoder_seek_proc onSeek, ma_decoder_tell_proc onTell, void* pUserData, const ma_decoder_config* pConfig, ma_decoder* pDecoder)
{
    ma_result result;
    ma_data_source_config dataSourceConfig;

    MA_ASSERT(pConfig != NULL);

    if (pDecoder == NULL) {
        return MA_INVALID_ARGS;
    }

    MA_ZERO_OBJECT(pDecoder);

    dataSourceConfig = ma_data_source_config_init();
    dataSourceConfig.vtable = &g_ma_decoder_data_source_vtable;

    result = ma_data_source_init(&dataSourceConfig, &pDecoder->ds);
    if (result != MA_SUCCESS) {
        return result;
    }

    pDecoder->onRead    = onRead;
    pDecoder->onSeek    = onSeek;
    pDecoder->onTell    = onTell;
    pDecoder->pUserData = pUserData;

    result = ma_decoder__init_allocation_callbacks(pConfig, pDecoder);
    if (result != MA_SUCCESS) {
        ma_data_source_uninit(&pDecoder->ds);
        return result;
    }

    return MA_SUCCESS;
}
```
here, first step is `MA_ZERO_OBJECT` to clear memory for pDecoder,
then here is the most important part:
```cpp
dataSourceConfig = ma_data_source_config_init();
dataSourceConfig.vtable = &g_ma_decoder_data_source_vtable;
```
these two step set the `ma_data_source` object ds within pDecoder. 
A dataSourceConfig and also, dataSource is something like that
```cpp
typedef void ma_data_source;

#define MA_DATA_SOURCE_SELF_MANAGED_RANGE_AND_LOOP_POINT    0x00000001

typedef struct
{
    ma_result (* onRead)(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead);
    ma_result (* onSeek)(ma_data_source* pDataSource, ma_uint64 frameIndex);
    ma_result (* onGetDataFormat)(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap);
    ma_result (* onGetCursor)(ma_data_source* pDataSource, ma_uint64* pCursor);
    ma_result (* onGetLength)(ma_data_source* pDataSource, ma_uint64* pLength);
    ma_result (* onSetLooping)(ma_data_source* pDataSource, ma_bool32 isLooping);
    ma_uint32 flags;
} ma_data_source_vtable;

typedef ma_data_source* (* ma_data_source_get_next_proc)(ma_data_source* pDataSource);

typedef struct
{
    const ma_data_source_vtable* vtable;
} ma_data_source_config;

MA_API ma_data_source_config ma_data_source_config_init(void);


typedef struct
{
    const ma_data_source_vtable* vtable;
    ma_uint64 rangeBegInFrames;
    ma_uint64 rangeEndInFrames;             /* Set to -1 for unranged (default). */
    ma_uint64 loopBegInFrames;              /* Relative to rangeBegInFrames. */
    ma_uint64 loopEndInFrames;              /* Relative to rangeBegInFrames. Set to -1 for the end of the range. */
    ma_data_source* pCurrent;               /* When non-NULL, the data source being initialized will act as a proxy and will route all operations to pCurrent. Used in conjunction with pNext/onGetNext for seamless chaining. */
    ma_data_source* pNext;                  /* When set to NULL, onGetNext will be used. */
    ma_data_source_get_next_proc onGetNext; /* Will be used when pNext is NULL. If both are NULL, no next will be used. */
    MA_ATOMIC(4, ma_bool32) isLooping;
} ma_data_source_base;

```
`ma_data_source` is an opaque type (via void), meaning it doesn't handle implementation details directly. 
Instead, it acts as a generic handle that informs the system: "Here is an object that behaves as a data source."
The ma_data_source_vtable is a collection of function pointers where the actual logic resides. 
When the system accesses the vtable, it performs a runtime jump to the specific memory address associated with each operation.
So essentially, this is a class implemented entirely in C using *manual polymorphism*.