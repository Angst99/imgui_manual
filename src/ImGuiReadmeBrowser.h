#pragma once
#include "source_parse/Sources.h"
#include "utilities/MarkdownHelper.h"

// This windows shows ImGui's Readme.md
class ImGuiReadmeBrowser
{
public:
    ImGuiReadmeBrowser() : mSource(SourceParse::ReadSource("imgui/README.md")) {}
    inline void gui()
    {
        MarkdownHelper::Markdown(mSource.sourceCode);
    }
private:
    SourceParse::Source mSource;
};
