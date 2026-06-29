#define MINIZ_IMPLEMENTATION
#define MINIZ_HEADER_FILE_ONLY

#include "Parser.h"

Parser::Parser(void) {}

int Parser::FindFileInDatapack(const std::string& FileName)
{
    for (int Index = 0; Index < this->ParserGroup.size(); Index++) {if (this->ParserGroup[Index].LabelName == FileName) {return Index;}}
    return -1;
}

std::vector<unsigned char> Parser::GetURIData(const std::string& ModelDataContents)
{
    std::stringstream NewStream;
    nlohmann::json BufferData   = nlohmann::json::parse(ModelDataContents);
    std::string BufferURI       = BufferData["buffers"][0]["uri"];
    std::string Data            = this->GetDatapackData("Models/"+BufferURI);
    std::vector<unsigned char> NewData(Data.begin(),Data.end());
    return NewData;
}

std::string Parser::GetDatapackData(const std::string& FileName)
{
    int FileIndex = this->FindFileInDatapack(FileName);
    if (FileIndex != -1)    {return this->ParserGroup[FileIndex].DataContents;}
    else                    {Console::SysPrint("Failed to get filename '"+FileName+"'.",2);}
    return "";
}

ImageFormat Parser::GetImageBuffer(const std::string& FileName)
{
    int FileIndex = this->FindFileInDatapack(FileName);
    if (FileIndex != -1)
    {
        ImageFormat NewFormat;
        NewFormat.ImageContents     = this->ParserGroup[FileIndex].ImageFormat.TextureData;
        NewFormat.NewBufferFormat   = this->ParserGroup[FileIndex].ImageFormat;
        NewFormat.FormatUsed        = true;
        return NewFormat;
    }
    Console::SysPrint("Failed to get image file "+FileName,2);
    return ImageFormat{};
}

FILE* Parser::LoadSceneBSPFile(const std::string& Datapack,const std::string& SceneFileName)
{
    char TemporaryFileName[L_tmpnam];
    mz_zip_archive NewArchive;
    memset(&NewArchive,0,sizeof(NewArchive));

    if (!mz_zip_reader_init_file(&NewArchive,Datapack.c_str(),0)) {Console::SysPrint("Failed to open '"+Datapack+"'.",3); return nullptr;}

    size_t SceneExtractedSize   = 0;
    void* FilePointer           = mz_zip_reader_extract_file_to_heap(&NewArchive,SceneFileName.c_str(),&SceneExtractedSize,0);
    mz_zip_reader_end(&NewArchive);

    if (!FilePointer) {Console::SysPrint("Failed to get file from Datapack ('"+Datapack+"' -> '"+SceneFileName+"'.",3); return nullptr;}

    FILE* NewTemporaryFilePointer = tmpfile();
    if (!NewTemporaryFilePointer) {Console::SysPrint("Failed to create a temporary file.",3); return nullptr;}

    fwrite(FilePointer,1,SceneExtractedSize,NewTemporaryFilePointer);
    free(FilePointer);

    rewind(NewTemporaryFilePointer);
    return NewTemporaryFilePointer;
}

bool Parser::LoadFromBuffer(const char* Buffer,size_t BufferSize,ImageBufferFormat* ImageFormatOut)
{
    const char* Pointer     = Buffer;
    const char* BufferEnd   = Buffer+BufferSize;
    unsigned int DataSize;

    auto ReadUnsigned = [&](unsigned int& Out) -> bool {
        if (BufferEnd-Pointer < sizeof(unsigned int)) {return false;}
        std::memcpy(&Out,Pointer,sizeof(unsigned int));
        Pointer += sizeof(unsigned int);
        return true;
    };

    if (!ReadUnsigned(ImageFormatOut->TextureWidth))    {Console::SysPrint("Corrupted buffer: missing width on image.",3); return false;}
    if (!ReadUnsigned(ImageFormatOut->TextureHeight))   {Console::SysPrint("Corrupted buffer: missing height on image.",3); return false;} 
    if (!ReadUnsigned(ImageFormatOut->TextureChannel))  {Console::SysPrint("Corrupted buffer: missing channel on image.",3); return false;}
    if (!ReadUnsigned(DataSize))                        {Console::SysPrint("Corrupted buffer: missing datasize on image.",3); return false;}

    if (BufferEnd-Pointer < static_cast<ptrdiff_t>(DataSize)) {Console::SysPrint("Corrupted buffer: not enough image data given declared size.",3); return false;}
    ImageFormatOut->TextureData.resize(DataSize);
    std::memcpy(ImageFormatOut->TextureData.data(),Pointer,DataSize);
    return true;
}

void Parser::FetchFromDatapack(const std::string& FilePath)
{
    std::vector<std::string> GroupFolders = {"Shader","Textures","Skybox","Models"};

    this->TimedMeasurement = glfwGetTime();
    memset(&this->NewDataparser,0,sizeof(this->NewDataparser));
    if (!mz_zip_reader_init_file(&this->NewDataparser,FilePath.c_str(),0)) {Console::SysPrint("Failed to open Datapack file '"+FilePath+"'.",3); return;}

    int FileCount = (int)mz_zip_reader_get_num_files(&this->NewDataparser);
    for (int Index = 0; Index < FileCount; Index++)
    {
        mz_zip_archive_file_stat NewFileStat;
        if (!mz_zip_reader_file_stat(&this->NewDataparser,Index,&NewFileStat)) {Console::SysPrint("Failed to get file data for index"+std::to_string(Index),1); continue;}

        std::string NewFileName(NewFileStat.m_filename);
        for (int FolderIndex = 0; FolderIndex < GroupFolders.size(); FolderIndex++)
        {
            if (NewFileName.rfind(GroupFolders[FolderIndex],0) == 0 && !NewFileStat.m_is_directory)
            {
                size_t NewSize = 0;
                ParserMap NewParserMap;
                Console::SysPrint("Reading file '"+std::string(NewFileStat.m_filename)+"' ("+std::to_string((unsigned int)NewFileStat.m_uncomp_size)+" bytes).",1);
                void* FileDataInMemory = mz_zip_reader_extract_to_heap(&this->NewDataparser,Index,&NewSize,0);

                if (!FileDataInMemory) {Console::SysPrint("Failed to open file "+std::string(NewFileStat.m_filename),2); continue;}

                std::string CurrentFileContents(static_cast<char*>(FileDataInMemory),NewSize);
                if (std::string(NewFileStat.m_filename).find(".imgbuf") != std::string::npos)
                {
                    ImageBufferFormat NewFormat;
                    if (this->LoadFromBuffer(CurrentFileContents.data(),CurrentFileContents.size(),&NewFormat))     {NewParserMap.ImageFormat = NewFormat;}
                    else                                                                                            {Console::SysPrint("Failed to get buffer data for '"+std::string(NewFileStat.m_filename)+"'.",3); continue;}
                }   

                NewParserMap.DataContents   = CurrentFileContents;
                NewParserMap.LabelName      = std::string(NewFileStat.m_filename);
                this->ParserGroup.push_back(NewParserMap);
            }
        }
    }
    mz_zip_reader_end(&this->NewDataparser);
    Console::SysPrint("Completed reading the datapack. (Read in "+std::to_string(glfwGetTime()-this->TimedMeasurement)+" ms)",1);
}

void Parser::CleanParser(void)
{
    for (int Index = 0; Index < this->ParserGroup.size(); Index++) {free(&this->ParserGroup[Index].ImageFormat.TextureData[0]);}
}
