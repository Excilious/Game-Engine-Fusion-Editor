#ifndef __PARSER_HEADER__
#define __PARSER_HEADER__

#include <map>
#include <vector>
#include <cstdint>
#include <string>
#include <fstream>
#include <cstring>

#include "../../Resources/zip/miniz.h"
#include "../../Resources/jsonimport/json.h"
#include "Console.h"
#include "Shader.h"

struct ImageBufferFormat
{
    unsigned int                TextureWidth;
    unsigned int                TextureHeight;
    unsigned int                TextureChannel;
    std::vector<unsigned char>  TextureData;
};

struct ImageFormat
{
    bool                        FormatUsed;
    std::vector<unsigned char>  ImageContents;
    ImageBufferFormat           NewBufferFormat;
};

typedef struct
{
    std::string         LabelName;
    std::string         DataContents;
    ImageBufferFormat   ImageFormat;
} ParserMap;

class Parser
{
    public:
                                        Parser(void);
        void                            CleanParser(void);
        void                            FetchFromDatapack(const std::string& FilePath);
        bool                            LoadFromBuffer(const char* Buffer,size_t BufferSize,ImageBufferFormat* ImageFormatOut);
        std::string                     GetDatapackData(const std::string& FileName);
        std::vector<unsigned char>      GetURIData(const std::string& ModelDataContents);
        ImageFormat                     GetImageBuffer(const std::string& FileName);
        FILE*                           LoadSceneBSPFile(const std::string& Datapack,const std::string& SceneFileName);
    private:
        int                             FindFileInDatapack(const std::string& FileName);
        mz_zip_archive                  NewDataparser;
        std::vector<ParserMap>          ParserGroup;
        double                          TimedMeasurement = 0;
};

#endif