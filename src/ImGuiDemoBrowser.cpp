#include "source_parse/ImGuiRepoUrl.h"

#include "imgui_utilities/HyperlinkHelper.h"
#include "imgui_utilities/ImGuiExt.h"
#include "source_parse/ImGuiDemoParser.h"

#include "hello_imgui/hello_imgui.h"

#include <fplus/fplus.hpp>

#include "ImGuiDemoBrowser.h"

// Redefinition of ImGuiDemoCallback, as defined in imgui_demo.cpp
typedef void (*ImGuiDemoCallback)(const char* file, int line_number, const char* demo_title, bool hovering_zone_only);
extern ImGuiDemoCallback gImGuiDemoCallback;

// implImGuiDemoCallbackDemoCallback is the implementation
// of imgui_demo.cpp's global callback (gImGuiDemoCallback)
// And  gImGuiDemoBrowser is a global reference to the browser used by this callback
ImGuiDemoBrowser *gImGuiDemoBrowser = nullptr;
extern HelloImGui::RunnerParams runnerParams; // defined in ImGuiManual.cpp
void implImGuiDemoCallbackDemoCallback(const char* file, int line_number, const char* /*demo_title*/, bool hovering_zone_only)
{
    if (hovering_zone_only)
    {
        gImGuiDemoBrowser->followDemo(line_number);
    }
    else
    {
        int cursorLineOnPage = 3;
        gImGuiDemoBrowser->_GetTextEditorPtr()->SetCursorPosition({line_number, 0}, cursorLineOnPage);
        runnerParams.dockingParams.focusDockableWindow("ImGui - Demo Code");
    }
}


ImGuiDemoBrowser::ImGuiDemoBrowser()
    : WindowWithEditor("ImGui - Demo Code")
    , mAnnotatedSource(SourceParse::ReadImGuiDemoCode())
    , mGuiHeaderTree(mAnnotatedSource.linesWithTags)
{
    setEditorAnnotatedSource(mAnnotatedSource);

    // Setup of imgui_demo.cpp's global callback
    // (gImGuiDemoCallback belongs to imgui.cpp!)
    gImGuiDemoCallback = implImGuiDemoCallbackDemoCallback;
    gImGuiDemoBrowser = this;
}

void ImGuiDemoBrowser::gui()
{
    guiHelp();
    guiDemoCodeTags();
    guiSave();
    RenderEditor("imgui_demo.cpp", [this] { this->guiGithubButton(); });
}

void ImGuiDemoBrowser::guiHelp()
{
    std::string help =
        "This is the code of imgui_demo.cpp. It is the best way to learn about Dear ImGui! \n"
        "\n"
        "* On the left, you can see a demo that showcases all the widgets and features of ImGui:\n"
        "  Click on the \"Code\" buttons in the demo to see the code corresponding to a given feature. \n"
        "\n"
        "* Below, the table of content (TOC) shows all the available demos: click on any item to see its code.\n"
        "  If 'Follow Demo' is checked, the TOC will follow the mouse whenever you hover a demo.\n"
        "  Alternatively, you can also search for some features (try searching for \"widgets\", \"layout\", \"drag\", etc)\n"
        ;
    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.f, 1.0f), "(?)");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", help.c_str());
    ImGui::SameLine(50.f);
}

void ImGuiDemoBrowser::guiSave()
{
#ifdef IMGUI_MANUAL_CAN_WRITE_IMGUI_DEMO_CPP
    if (ImGui::Button("Save"))
    {
        std::string fileSrc = IMGUI_MANUAL_REPO_DIR "/external/imgui/imgui_demo.cpp";
        fplus::write_text_file(fileSrc, mEditor.GetText())();
    }
    ImGui::SameLine();
#endif
}

void ImGuiDemoBrowser::guiGithubButton()
{
    if (ImGui::SmallButton("View on github at this line"))
    {
        std::string url = ImGuiRepoUrl() + "imgui_demo.cpp#L"
                          + std::to_string(mEditor.GetCursorPosition().mLine + 1);
        HyperlinkHelper::OpenUrl(url);
    }
}

void ImGuiDemoBrowser::guiDemoCodeTags()
{
    int currentEditorLineNumber = mEditor.GetCursorPosition().mLine;
    int selectedLine = mGuiHeaderTree.gui(currentEditorLineNumber);
    if (selectedLine >= 0)
        mEditor.SetCursorPosition({selectedLine, 0}, 3);
}

void ImGuiDemoBrowser::followDemo(int sourceLineNumber)
{
    if (mGuiHeaderTree.isFollowActive())
    {
        mGuiHeaderTree.followShowTocElementForLine(sourceLineNumber);
        mEditor.SetCursorPosition({sourceLineNumber, 0}, 3);
    }

}
