#include "hello_imgui/hello_imgui.h"
#include "imgui.h"


enum MyItemColumnID {
    MyItemColumnID_ID,
    MyItemColumnID_Name,
    MyItemColumnID_Action,
    MyItemColumnID_Quantity,
    MyItemColumnID_Description
};

struct MyItem {
    int ID;
    const char *Name;
    int Quantity;
    static inline const ImGuiTableSortSpecs *s_current_sort_specs;

    static void SortWithSortSpecs(ImGuiTableSortSpecs *sort_specs, MyItem *items, int items_count) {
        s_current_sort_specs = sort_specs;// Store in variable accessible by the sort function.
        if (items_count > 1)
            qsort(items, (size_t) items_count, sizeof(items[0]), MyItem::CompareWithSortSpecs);
        s_current_sort_specs = NULL;
    }

    // Compare function to be used by qsort()
    static int CompareWithSortSpecs(const void *lhs, const void *rhs) {
        const MyItem *a = (const MyItem *) lhs;
        const MyItem *b = (const MyItem *) rhs;
        for (int n = 0; n < s_current_sort_specs->SpecsCount; n++) {
            // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
            // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
            const ImGuiTableColumnSortSpecs *sort_spec = &s_current_sort_specs->Specs[n];
            int delta = 0;
            switch (sort_spec->ColumnUserID) {
                case MyItemColumnID_ID:
                    delta = (a->ID - b->ID);
                    break;
                case MyItemColumnID_Name:
                    delta = (strcmp(a->Name, b->Name));
                    break;
                case MyItemColumnID_Quantity:
                    delta = (a->Quantity - b->Quantity);
                    break;
                case MyItemColumnID_Description:
                    delta = (strcmp(a->Name, b->Name));
                    break;
                default:
                    IM_ASSERT(0);
                    break;
            }
            if (delta > 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
        }

        // qsort() is instable so always return a way to differenciate items.
        // Your own compare function may want to avoid fallback on implicit sort specs.
        // e.g. a Name compare if it wasn't already part of the sort specs.
        return (a->ID - b->ID);
    }
};

static void PushStyleCompact() {
    ImGuiStyle &style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float) (int) (style.FramePadding.y * 0.60f)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float) (int) (style.ItemSpacing.y * 0.60f)));
}
static void HelpMarker(const char *desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// Show a combo box with a choice of sizing policies
static void EditTableSizingFlags(ImGuiTableFlags *p_flags) {
    struct EnumDesc {
        ImGuiTableFlags Value;
        const char *Name;
        const char *Tooltip;
    };
    static const EnumDesc policies[] =
        {
            {ImGuiTableFlags_None, "Default", "Use default sizing policy:\n- ImGuiTableFlags_SizingFixedFit if ScrollX is on or if host window has ImGuiWindowFlags_AlwaysAutoResize.\n- ImGuiTableFlags_SizingStretchSame otherwise."},
            {ImGuiTableFlags_SizingFixedFit, "ImGuiTableFlags_SizingFixedFit", "Columns default to _WidthFixed (if resizable) or _WidthAuto (if not resizable), matching contents width."},
            {ImGuiTableFlags_SizingFixedSame, "ImGuiTableFlags_SizingFixedSame", "Columns are all the same width, matching the maximum contents width.\nImplicitly disable ImGuiTableFlags_Resizable and enable ImGuiTableFlags_NoKeepColumnsVisible."},
            {ImGuiTableFlags_SizingStretchProp, "ImGuiTableFlags_SizingStretchProp", "Columns default to _WidthStretch with weights proportional to their widths."},
            {ImGuiTableFlags_SizingStretchSame, "ImGuiTableFlags_SizingStretchSame", "Columns default to _WidthStretch with same weights."}};
    int idx;
    for (idx = 0; idx < IM_ARRAYSIZE(policies); idx++)
        if (policies[idx].Value == (*p_flags & ImGuiTableFlags_SizingMask_))
            break;
    const char *preview_text = (idx < IM_ARRAYSIZE(policies)) ? policies[idx].Name + (idx > 0 ? strlen("ImGuiTableFlags") : 0) : "";
    if (ImGui::BeginCombo("Sizing Policy", preview_text)) {
        for (int n = 0; n < IM_ARRAYSIZE(policies); n++)
            if (ImGui::Selectable(policies[n].Name, idx == n))
                *p_flags = (*p_flags & ~ImGuiTableFlags_SizingMask_) | policies[n].Value;
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 50.0f);
        for (int m = 0; m < IM_ARRAYSIZE(policies); m++) {
            ImGui::Separator();
            ImGui::Text("%s:", policies[m].Name);
            ImGui::Separator();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().IndentSpacing * 0.5f);
            ImGui::TextUnformatted(policies[m].Tooltip);
        }
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
static void PopStyleCompact() {
    ImGui::PopStyleVar(2);
}

int main(int, char *[]) {
    HelloImGui::RunnerParams runnerParams;// runnerParams will contains all the application params and callbacks
    runnerParams.callbacks.ShowGui =      // ShowGui contains a lambda function with the Gui code
        [&runnerParams] {
            int screenWidth = 1280;
            int screenHeight = 720;
            float windowWidth = screenWidth - screenWidth / 5;
            float windowHeight = screenHeight - screenHeight / 5;
            float x = (screenWidth - windowWidth) / 2.0f;
            float y = (screenHeight - windowHeight) / 2.0f;
            ImGui::SetNextWindowPos(ImVec2(x, y));
            ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

            ImGui::Begin("demo");


            // Demonstrate using Sorting facilities
            // This is a simplified version of the "Advanced" example, where we mostly focus on the code necessary to handle sorting.
            // Note that the "Advanced" example also showcase manually triggering a sort (e.g. if item quantities have been modified)
            static const char *template_items_names[] =
                {
                    "Banana", "Apple", "Cherry", "Watermelon", "Grapefruit", "Strawberry", "Mango",
                    "Kiwi", "Orange", "Pineapple", "Blueberry", "Plum", "Coconut", "Pear", "Apricot"};

            // IMGUI_DEMO_MARKER("Tables/Sorting");
            const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
            const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();


            {
                static ImGuiTableFlags flags =
                    ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
                static ImGuiTableColumnFlags columns_base_flags = ImGuiTableColumnFlags_None;

                enum ContentsType { CT_Text,
                                    CT_Button,
                                    CT_SmallButton,
                                    CT_FillButton,
                                    CT_Selectable,
                                    CT_SelectableSpanRow };
                static int contents_type = CT_SelectableSpanRow;
                const char *contents_type_names[] = {"Text", "Button", "SmallButton", "FillButton", "Selectable", "Selectable (span row)"};
                static int freeze_cols = 1;
                static int freeze_rows = 1;
                static int items_count = IM_ARRAYSIZE(template_items_names) * 2;
                static ImVec2 outer_size_value = ImVec2(0.0f, TEXT_BASE_HEIGHT * 12);
                static float row_min_height = 0.0f;         // Auto
                static float inner_width_with_scroll = 0.0f;// Auto-extend
                static bool outer_size_enabled = true;
                static bool show_headers = true;
                static bool show_wrapped_text = false;
                //static ImGuiTextFilter filter;
                //ImGui::SetNextItemOpen(true, ImGuiCond_Once); // FIXME-TABLE: Enabling this results in initial clipped first pass on table which tend to affect column sizing

                ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
                if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
                    if (ImGui::BeginTabItem("Broccoli")) {
                        ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Avocado")) {

                        ImGui::Separator();

                        // const char *buttonLabels[] = {"Button 1", "Button 2", "Button 3", "Button 4", "Button 5", "Button 6", "Button 7", "Button 8"};
                        // int buttonCount = sizeof(buttonLabels) / sizeof(buttonLabels[0]);
                        // float windowWidth = ImGui::GetContentRegionAvail().x;
                        // const float buttonSpacing = 10.0f;
                        // float buttonWidth = (windowWidth - (buttonCount - 1) * buttonSpacing) / buttonCount;

                        // for (int i = 0; i < buttonCount; ++i) {
                        //     ImGui::PushItemWidth(buttonWidth);
                        //     if (ImGui::Button(buttonLabels[i])) {
                        //     }
                        //     ImGui::PopItemWidth();

                        //     if (i < buttonCount - 1) {
                        //         ImGui::SameLine(0.0f, buttonSpacing);
                        //     }
                        // }
                        const char *buttonLabels[] = {"Button 1", "Button 2", "Button 3", "Button 4", "Button 5", "Button 6", "Button 7", "Button 8"};
                        int buttonCount = sizeof(buttonLabels) / sizeof(buttonLabels[0]);

                        float windowWidth = ImGui::GetContentRegionAvail().x;
                        const float totalButtonWidth = windowWidth - ImGui::GetStyle().ItemSpacing.x * (buttonCount - 1);
                        float buttonWidth = totalButtonWidth / buttonCount;
                        const float buttonSpacing = ImGui::GetStyle().ItemSpacing.x;

                        for (int i = 0; i < buttonCount; ++i) {
                            if (i == buttonCount - 1) {
                                buttonWidth = windowWidth - (buttonWidth + buttonSpacing) * (buttonCount - 1);// 计算最后一个按钮的宽度，填充剩余空间
                            }

                            if (i > 0) {
                                ImGui::SameLine(0.0f, buttonSpacing);// 按钮间的间距
                            }

                            if (ImGui::Button(buttonLabels[i], ImVec2(buttonWidth, 0))) {
                                // 点击按钮后的处理
                            }
                        }


                        ImGui::Separator();


                        if (ImGui::TreeNode("Options")) {
                            // Make the UI compact because there are so many fields
                            PushStyleCompact();
                            ImGui::PushItemWidth(TEXT_BASE_WIDTH * 28.0f);

                            if (ImGui::TreeNodeEx("Features:", ImGuiTreeNodeFlags_DefaultOpen)) {
                                ImGui::CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
                                ImGui::CheckboxFlags("ImGuiTableFlags_Reorderable", &flags, ImGuiTableFlags_Reorderable);
                                ImGui::CheckboxFlags("ImGuiTableFlags_Hideable", &flags, ImGuiTableFlags_Hideable);
                                ImGui::CheckboxFlags("ImGuiTableFlags_Sortable", &flags, ImGuiTableFlags_Sortable);
                                ImGui::CheckboxFlags("ImGuiTableFlags_NoSavedSettings", &flags, ImGuiTableFlags_NoSavedSettings);
                                ImGui::CheckboxFlags("ImGuiTableFlags_ContextMenuInBody", &flags, ImGuiTableFlags_ContextMenuInBody);
                                ImGui::TreePop();
                            }

                            if (ImGui::TreeNodeEx("Decorations:", ImGuiTreeNodeFlags_DefaultOpen)) {
                                ImGui::CheckboxFlags("ImGuiTableFlags_RowBg", &flags, ImGuiTableFlags_RowBg);
                                ImGui::CheckboxFlags("ImGuiTableFlags_BordersV", &flags, ImGuiTableFlags_BordersV);
                                ImGui::CheckboxFlags("ImGuiTableFlags_BordersOuterV", &flags, ImGuiTableFlags_BordersOuterV);
                                ImGui::CheckboxFlags("ImGuiTableFlags_BordersInnerV", &flags, ImGuiTableFlags_BordersInnerV);
                                ImGui::CheckboxFlags("ImGuiTableFlags_BordersH", &flags, ImGuiTableFlags_BordersH);
                                ImGui::CheckboxFlags("ImGuiTableFlags_BordersOuterH", &flags, ImGuiTableFlags_BordersOuterH);
                                ImGui::CheckboxFlags("ImGuiTableFlags_BordersInnerH", &flags, ImGuiTableFlags_BordersInnerH);
                                ImGui::CheckboxFlags("ImGuiTableFlags_NoBordersInBody", &flags, ImGuiTableFlags_NoBordersInBody);
                                ImGui::SameLine();
                                HelpMarker("Disable vertical borders in columns Body (borders will always appear in Headers");
                                ImGui::CheckboxFlags("ImGuiTableFlags_NoBordersInBodyUntilResize", &flags, ImGuiTableFlags_NoBordersInBodyUntilResize);
                                ImGui::SameLine();
                                HelpMarker("Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers)");
                                ImGui::TreePop();
                            }

                            if (ImGui::TreeNodeEx("Sizing:", ImGuiTreeNodeFlags_DefaultOpen)) {
                                EditTableSizingFlags(&flags);
                                ImGui::SameLine();
                                HelpMarker("In the Advanced demo we override the policy of each column so those table-wide settings have less effect that typical.");
                                ImGui::CheckboxFlags("ImGuiTableFlags_NoHostExtendX", &flags, ImGuiTableFlags_NoHostExtendX);
                                ImGui::SameLine();
                                HelpMarker("Make outer width auto-fit to columns, overriding outer_size.x value.\n\nOnly available when ScrollX/ScrollY are disabled and Stretch columns are not used.");
                                ImGui::CheckboxFlags("ImGuiTableFlags_NoHostExtendY", &flags, ImGuiTableFlags_NoHostExtendY);
                                ImGui::SameLine();
                                HelpMarker("Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit).\n\nOnly available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.");
                                ImGui::CheckboxFlags("ImGuiTableFlags_NoKeepColumnsVisible", &flags, ImGuiTableFlags_NoKeepColumnsVisible);
                                ImGui::SameLine();
                                HelpMarker("Only available if ScrollX is disabled.");
                                ImGui::CheckboxFlags("ImGuiTableFlags_PreciseWidths", &flags, ImGuiTableFlags_PreciseWidths);
                                ImGui::SameLine();
                                HelpMarker("Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.");
                                ImGui::CheckboxFlags("ImGuiTableFlags_NoClip", &flags, ImGuiTableFlags_NoClip);
                                ImGui::SameLine();
                                HelpMarker("Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with ScrollFreeze options.");
                                ImGui::TreePop();
                            }

                            if (ImGui::TreeNodeEx("Padding:", ImGuiTreeNodeFlags_DefaultOpen)) {
                                ImGui::CheckboxFlags("ImGuiTableFlags_PadOuterX", &flags, ImGuiTableFlags_PadOuterX);
                                ImGui::CheckboxFlags("ImGuiTableFlags_NoPadOuterX", &flags, ImGuiTableFlags_NoPadOuterX);
                                ImGui::CheckboxFlags("ImGuiTableFlags_NoPadInnerX", &flags, ImGuiTableFlags_NoPadInnerX);
                                ImGui::TreePop();
                            }

                            if (ImGui::TreeNodeEx("Scrolling:", ImGuiTreeNodeFlags_DefaultOpen)) {
                                ImGui::CheckboxFlags("ImGuiTableFlags_ScrollX", &flags, ImGuiTableFlags_ScrollX);
                                ImGui::SameLine();
                                ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
                                ImGui::DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
                                ImGui::CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
                                ImGui::SameLine();
                                ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
                                ImGui::DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
                                ImGui::TreePop();
                            }

                            if (ImGui::TreeNodeEx("Sorting:", ImGuiTreeNodeFlags_DefaultOpen)) {
                                ImGui::CheckboxFlags("ImGuiTableFlags_SortMulti", &flags, ImGuiTableFlags_SortMulti);
                                ImGui::SameLine();
                                HelpMarker("When sorting is enabled: hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).");
                                ImGui::CheckboxFlags("ImGuiTableFlags_SortTristate", &flags, ImGuiTableFlags_SortTristate);
                                ImGui::SameLine();
                                HelpMarker("When sorting is enabled: allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).");
                                ImGui::TreePop();
                            }

                            if (ImGui::TreeNodeEx("Headers:", ImGuiTreeNodeFlags_DefaultOpen)) {
                                ImGui::Checkbox("show_headers", &show_headers);
                                ImGui::CheckboxFlags("ImGuiTableFlags_HighlightHoveredColumn", &flags, ImGuiTableFlags_HighlightHoveredColumn);
                                ImGui::CheckboxFlags("ImGuiTableColumnFlags_AngledHeader", &columns_base_flags, ImGuiTableColumnFlags_AngledHeader);
                                ImGui::SameLine();
                                HelpMarker("Enable AngledHeader on all columns. Best enabled on selected narrow columns (see \"Angled headers\" section of the demo).");
                                ImGui::TreePop();
                            }

                            if (ImGui::TreeNodeEx("Other:", ImGuiTreeNodeFlags_DefaultOpen)) {
                                ImGui::Checkbox("show_wrapped_text", &show_wrapped_text);

                                ImGui::DragFloat2("##OuterSize", &outer_size_value.x);
                                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                                ImGui::Checkbox("outer_size", &outer_size_enabled);
                                ImGui::SameLine();
                                HelpMarker("If scrolling is disabled (ScrollX and ScrollY not set):\n"
                                           "- The table is output directly in the parent window.\n"
                                           "- OuterSize.x < 0.0f will right-align the table.\n"
                                           "- OuterSize.x = 0.0f will narrow fit the table unless there are any Stretch columns.\n"
                                           "- OuterSize.y then becomes the minimum size for the table, which will extend vertically if there are more rows (unless NoHostExtendY is set).");

                                // From a user point of view we will tend to use 'inner_width' differently depending on whether our table is embedding scrolling.
                                // To facilitate toying with this demo we will actually pass 0.0f to the BeginTable() when ScrollX is disabled.
                                ImGui::DragFloat("inner_width (when ScrollX active)", &inner_width_with_scroll, 1.0f, 0.0f, FLT_MAX);

                                ImGui::DragFloat("row_min_height", &row_min_height, 1.0f, 0.0f, FLT_MAX);
                                ImGui::SameLine();
                                HelpMarker("Specify height of the Selectable item.");

                                ImGui::DragInt("items_count", &items_count, 0.1f, 0, 9999);
                                ImGui::Combo("items_type (first column)", &contents_type, contents_type_names, IM_ARRAYSIZE(contents_type_names));
                                //filter.Draw("filter");
                                ImGui::TreePop();
                            }

                            ImGui::PopItemWidth();
                            PopStyleCompact();
                            ImGui::Spacing();
                            ImGui::TreePop();
                        }

                        // Update item list if we changed the number of items
                        static ImVector<MyItem> items;
                        static ImVector<int> selection;
                        static bool items_need_sort = false;
                        if (items.Size != items_count) {
                            items.resize(items_count, MyItem());
                            for (int n = 0; n < items_count; n++) {
                                const int template_n = n % IM_ARRAYSIZE(template_items_names);
                                MyItem &item = items[n];
                                item.ID = n;
                                item.Name = template_items_names[template_n];
                                item.Quantity = (template_n == 3) ? 10 : (template_n == 4) ? 20
                                                                                           : 0;// Assign default quantities
                            }
                        }

                        const ImDrawList *parent_draw_list = ImGui::GetWindowDrawList();
                        const int parent_draw_list_draw_cmd_count = parent_draw_list->CmdBuffer.Size;
                        ImVec2 table_scroll_cur, table_scroll_max;// For debug display
                        const ImDrawList *table_draw_list = NULL; // "

                        // Submit table
                        const float inner_width_to_use = (flags & ImGuiTableFlags_ScrollX) ? inner_width_with_scroll : 0.0f;
                        if (ImGui::BeginTable("table_advanced", 6, flags, outer_size_enabled ? outer_size_value : ImVec2(0, 0), inner_width_to_use)) {
                            // Declare columns
                            // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
                            // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
                            ImGui::TableSetupColumn("ID", columns_base_flags | ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, MyItemColumnID_ID);
                            ImGui::TableSetupColumn("Name", columns_base_flags | ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Name);
                            ImGui::TableSetupColumn("Action", columns_base_flags | ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Action);
                            ImGui::TableSetupColumn("Quantity", columns_base_flags | ImGuiTableColumnFlags_PreferSortDescending, 0.0f, MyItemColumnID_Quantity);
                            ImGui::TableSetupColumn("Description", columns_base_flags | ((flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch), 0.0f, MyItemColumnID_Description);
                            ImGui::TableSetupColumn("Hidden", columns_base_flags | ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoSort);
                            ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);

                            // Sort our data if sort specs have been changed!
                            ImGuiTableSortSpecs *sort_specs = ImGui::TableGetSortSpecs();
                            if (sort_specs && sort_specs->SpecsDirty)
                                items_need_sort = true;
                            if (sort_specs && items_need_sort && items.Size > 1) {
                                MyItem::SortWithSortSpecs(sort_specs, items.Data, items.Size);
                                sort_specs->SpecsDirty = false;
                            }
                            items_need_sort = false;

                            // Take note of whether we are currently sorting based on the Quantity field,
                            // we will use this to trigger sorting when we know the data of this column has been modified.
                            const bool sorts_specs_using_quantity = (ImGui::TableGetColumnFlags(3) & ImGuiTableColumnFlags_IsSorted) != 0;

                            // Show headers
                            if (show_headers && (columns_base_flags & ImGuiTableColumnFlags_AngledHeader) != 0)
                                ImGui::TableAngledHeadersRow();
                            if (show_headers)
                                ImGui::TableHeadersRow();

                            // Show data
                            // FIXME-TABLE FIXME-NAV: How we can get decent up/down even though we have the buttons here?
                            ImGui::PushButtonRepeat(true);
                            ImGuiListClipper clipper;
                            clipper.Begin(items.Size);
                            while (clipper.Step()) {
                                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)

                                {
                                    MyItem *item = &items[row_n];
                                    //if (!filter.PassFilter(item->Name))
                                    //    continue;

                                    const bool item_is_selected = selection.contains(item->ID);
                                    ImGui::PushID(item->ID);
                                    ImGui::TableNextRow(ImGuiTableRowFlags_None, row_min_height);

                                    // For the demo purpose we can select among different type of items submitted in the first column
                                    ImGui::TableSetColumnIndex(0);
                                    bool is_checkbox_selected = item_is_selected;
                                    ImGui::Checkbox("##Checkbox", &is_checkbox_selected);
                                    ImGui::SameLine();
                                    char label[32];
                                    sprintf(label, "%04d", item->ID);
                                    if (contents_type == CT_Text)
                                        ImGui::TextUnformatted(label);
                                    else if (contents_type == CT_Button)
                                        ImGui::Button(label);
                                    else if (contents_type == CT_SmallButton)
                                        ImGui::SmallButton(label);
                                    else if (contents_type == CT_FillButton)
                                        ImGui::Button(label, ImVec2(-FLT_MIN, 0.0f));
                                    else if (contents_type == CT_Selectable || contents_type == CT_SelectableSpanRow) {

                                        if (is_checkbox_selected != item_is_selected) {
                                            if (is_checkbox_selected) {
                                                selection.push_back(item->ID);
                                            } else {
                                                selection.find_erase_unsorted(item->ID);
                                            }
                                        }
                                        // ImGui::Selectable(label, item_is_selected,  ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, row_min_height));
                                        if (item_is_selected) {
                                            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
                                        }

                                        ImGui::Selectable(label, item_is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, row_min_height));

                                        if (item_is_selected) {
                                            ImGui::PopStyleColor();
                                        }
                                    }


                                    if (ImGui::TableSetColumnIndex(1))
                                        ImGui::TextUnformatted(item->Name);

                                    // Here we demonstrate marking our data set as needing to be sorted again if we modified a quantity,
                                    // and we are currently sorting on the column showing the Quantity.
                                    // To avoid triggering a sort while holding the button, we only trigger it when the button has been released.
                                    // You will probably need some extra logic if you want to automatically sort when a specific entry changes.
                                    if (ImGui::TableSetColumnIndex(2)) {
                                        if (ImGui::SmallButton("Chop")) { item->Quantity += 1; }
                                        if (sorts_specs_using_quantity && ImGui::IsItemDeactivated()) { items_need_sort = true; }
                                        ImGui::SameLine();
                                        if (ImGui::SmallButton("Eat")) { item->Quantity -= 1; }
                                        if (sorts_specs_using_quantity && ImGui::IsItemDeactivated()) { items_need_sort = true; }
                                        ImGui::SameLine();
                                        static int selected_fish = -1;
                                        const char *names[] = {"Bream", "Haddock", "Mackerel", "Pollock", "Tilefish"};
                                        static bool toggles[] = {true, false, false, false, false};

                                        if (ImGui::SmallButton("Select..")) {
                                            // 获取按钮的位置和大小
                                            ImVec2 button_pos = ImGui::GetItemRectMin();
                                            ImVec2 button_size = ImGui::GetItemRectSize();

                                            // 计算弹出窗口的位置，使其位于按钮的中间
                                            ImVec2 popup_pos = ImVec2(button_pos.x + button_size.x * 0.5f, button_pos.y + button_size.y * 0.5f);

                                            // 设置下一个窗口的位置为计算出的位置
                                            ImGui::SetNextWindowPos(popup_pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

                                            // 打开弹出窗口
                                            ImGui::OpenPopup("my_select_popup");
                                        }
                                        ImGui::SameLine();
                                        ImGui::TextUnformatted(selected_fish == -1 ? "<None>" : names[selected_fish]);
                                        if (ImGui::BeginPopup("my_select_popup", ImGuiWindowFlags_NoMove)) {
                                            // ImGui::SeparatorText("Aquarium");

                                            ImGui::Text("Aquarium");
                                            ImGui::SameLine(ImGui::GetWindowWidth() - 30);// 将关闭按钮放在标题栏的右侧
                                            if (ImGui::Button("X", ImVec2(20, 20)))
                                                ImGui::CloseCurrentPopup();// 关闭当前的弹出窗口

                                            ImGui::Separator();
                                            // ImGui::BeginTooltip();
                                            ImGui::ProgressBar(sinf((float) ImGui::GetTime()) * 0.5f + 0.5f, ImVec2(ImGui::GetFontSize() * 25, 0.0f));
                                            // ImGui::EndTooltip();

                                            for (int i = 0; i < IM_ARRAYSIZE(names); i++)
                                                if (ImGui::Selectable(names[i]))
                                                    selected_fish = i;
                                            ImGui::EndPopup();
                                        }
                                        if (sorts_specs_using_quantity && ImGui::IsItemDeactivated()) { items_need_sort = true; }
                                    }

                                    if (ImGui::TableSetColumnIndex(3))
                                        ImGui::Text("%d", item->Quantity);

                                    ImGui::TableSetColumnIndex(4);
                                    if (show_wrapped_text)
                                        ImGui::TextWrapped("Lorem ipsum dolor sit amet");
                                    else
                                        ImGui::Text("Lorem ipsum dolor sit amet");

                                    if (ImGui::TableSetColumnIndex(5))
                                        ImGui::Text("1234");

                                    ImGui::PopID();
                                }
                            }
                            ImGui::PopButtonRepeat();

                            // Store some info to display debug details below
                            table_scroll_cur = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
                            table_scroll_max = ImVec2(ImGui::GetScrollMaxX(), ImGui::GetScrollMaxY());
                            table_draw_list = ImGui::GetWindowDrawList();
                            ImGui::EndTable();
                        }
                        static bool show_debug_details = false;
                        ImGui::Checkbox("Debug details", &show_debug_details);
                        if (show_debug_details && table_draw_list) {
                            ImGui::SameLine(0.0f, 0.0f);
                            const int table_draw_list_draw_cmd_count = table_draw_list->CmdBuffer.Size;
                            if (table_draw_list == parent_draw_list)
                                ImGui::Text(": DrawCmd: +%d (in same window)",
                                            table_draw_list_draw_cmd_count - parent_draw_list_draw_cmd_count);
                            else
                                ImGui::Text(": DrawCmd: +%d (in child window), Scroll: (%.f/%.f) (%.f/%.f)",
                                            table_draw_list_draw_cmd_count - 1, table_scroll_cur.x, table_scroll_max.x, table_scroll_cur.y, table_scroll_max.y);
                        }

                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Broccoli")) {
                        ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Cucumber")) {
                        ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }

                ImGui::Separator();
            }

            ImGui::PopID();


            // if (disable_indent)
            //     ImGui::PopStyleVar();


            // for (int i = 0; i < 20; i++) {
            //     ImGui::Text("this is %d  Text!", i);
            // }
            ImGui::End();
        };

    runnerParams.appWindowParams.windowTitle = "imgui_demo";
    // runnerParams.appWindowParams.windowGeometry.size = {1280, 720};
    runnerParams.appWindowParams.windowGeometry.size = {2400, 1080};
    HelloImGui::Run(runnerParams);
    return 0;
}
