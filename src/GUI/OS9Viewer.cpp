#include "OS9Viewer.hpp"
#include "MainFrame.hpp"
#include "../CDI/common/utils.hpp"

#include <wx/choicebk.h>
#include <wx/panel.h>
#include <wx/propgrid/propgrid.h>

static void addProperty(wxPropertyGrid* properties, wxPGProperty* property)
{
    property->ChangeFlag(wxPG_PROP_READONLY, true);
    // prop->Enable(false);
    properties->Append(property);
}

static void addUIntProperty(wxPropertyGrid* properties, const std::string& label, const std::string& name, unsigned long value)
{
    addProperty(properties, new wxUIntProperty(label, name, value));
}

static void addHexProperty(wxPropertyGrid* properties, const std::string& label, const std::string& name, unsigned long value)
{
    // TODO: std::format.
    addProperty(properties, new wxStringProperty(label, name, toHex(value)));
}

static void addExtraHeaderProperties(wxPropertyGrid* properties, const OS9::ModuleHeader& header)
{
    // TODO: find a way to keep the module order.
    switch(header.M_Type)
    {
    case OS9::ModuleHeader::TrapLib:
        addHexProperty(properties, "Init", "init", header.extra.M_Init);
        addHexProperty(properties, "Term", "term", header.extra.M_Term);
        [[fallthrough]];

    case OS9::ModuleHeader::Program:
        addHexProperty(properties, "Stack", "stack", header.extra.M_Stack);
        addHexProperty(properties, "IData", "idata", header.extra.M_IData);
        addHexProperty(properties, "IRefs", "irefs", header.extra.M_IRefs);
        [[fallthrough]];

    case OS9::ModuleHeader::Driver:
        addUIntProperty(properties, "Mem", "mem", header.extra.M_Mem);
        [[fallthrough]];

    case OS9::ModuleHeader::FileManager:
    case OS9::ModuleHeader::System:
        addHexProperty(properties, "Exec", "exec", header.extra.M_Exec);
        addHexProperty(properties, "Excpt", "excpt", header.extra.M_Excpt);
        break;

    default:
        break;
    }
}

/** \brief OS-9 Viewer.
 * Requires the \p cedimu.m_cdi to be a valid pointer.
 *
 * TODO: when deleting the core, what to do?
 */
OS9Viewer::OS9Viewer(MainFrame* mainFrame, CeDImu& cedimu)
    : wxFrame(mainFrame, wxID_ANY, "OS-9 Viewer")
    , m_cedimu(cedimu)
    , m_mainFrame(mainFrame)
{
    m_auiNotebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                      wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS);

    // Module list
    {
        wxChoicebook* choicebook = new wxChoicebook(m_auiNotebook, wxID_ANY);

        std::lock_guard<std::recursive_mutex> lock(m_cedimu.m_cdiMutex);
        for(const OS9::ModuleHeader& header : m_cedimu.m_cdi->GetBIOS().GetModules())
        {
            wxPropertyGrid* properties = new wxPropertyGrid(choicebook);

            const uint32_t base = m_cedimu.m_cdi->GetBIOSBaseAddress();
            addHexProperty(properties, "Address", "address", base + header.begin);
            addHexProperty(properties, "Execution", "execution", base + header.begin + header.extra.M_Exec);
            addHexProperty(properties, "Exception", "exception", base + header.begin + header.extra.M_Excpt);

            // Header
            properties->Append(new wxPropertyCategory("Header"));
            addUIntProperty(properties, "Size", "size", header.M_Size);
            addHexProperty(properties, "Owner", "owner", header.M_Owner);
            addHexProperty(properties, "Name", "name", header.M_Name);
            addHexProperty(properties, "Access", "access", header.M_Accs);
            addUIntProperty(properties, "Type", "type", header.M_Type);
            addUIntProperty(properties, "Lang", "lang", header.M_Lang);
            addHexProperty(properties, "Attr", "attr", header.M_Attr);
            addUIntProperty(properties, "Revs", "revs", header.M_Revs);

            // Extra header
            properties->Append(new wxPropertyCategory("Extra header"));
            addExtraHeaderProperties(properties, header);

            choicebook->AddPage(properties, header.name);
        }

        m_auiNotebook->AddPage(choicebook, "Modules");
    }

    Show();
}

OS9Viewer::~OS9Viewer()
{
    m_mainFrame->m_os9Viewer = nullptr;
}
