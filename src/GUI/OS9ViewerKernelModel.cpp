#include "OS9ViewerKernelModel.hpp"
#include "../CDI/common/utils.hpp"

#define ITEM_ID_TO_NODE(item) (static_cast<OS9ViewerKernelModelNode*>(item.GetID()))

OS9ViewerKernelModelNode::OS9ViewerKernelModelNode(OS9ViewerKernelModelNode* parent, const std::string& containerName, const std::string& displayName)
    : m_member{containerName}
    , m_value{}
    , m_name{displayName}
    , m_isContainer{true}
    , m_parent{parent}
{}

OS9ViewerKernelModelNode::OS9ViewerKernelModelNode(OS9ViewerKernelModelNode* parent, const std::string& itemName, const std::string& itemValue, const std::string& displayName)
    : m_member{itemName}
    , m_value{itemValue}
    , m_name{displayName}
    , m_isContainer{false}
    , m_parent{parent}
{}

/** \brief Contructs in this node the child (without the parent pointer which is automatically added).
 * \return The newly constructed child.
 */
OS9ViewerKernelModelNode* OS9ViewerKernelModelNode::AddContainer(const std::string& containerName, const std::string& displayName)
{
    return AddChildren(containerName, displayName);
}

OS9ViewerKernelModelNode* OS9ViewerKernelModelNode::AddItem(const std::string& itemName, const std::string& itemValue, const std::string& displayName)
{
    return AddChildren(itemName, itemValue, displayName);
}

template<typename... Args>
OS9ViewerKernelModelNode* OS9ViewerKernelModelNode::AddChildren(Args&&... args)
{
    m_children.emplace_back(std::make_unique<OS9ViewerKernelModelNode, OS9ViewerKernelModelNode*, Args...>(this, args...));
    return m_children.rbegin()->get();
}

OS9ViewerKernelModelNodePtr OS9ViewerKernelModel::BuildRootNode()
{
    const OS9::Kernel* kernel = m_cedimu.m_cdi->GetKernel();
    if(kernel == nullptr)
        return OS9ViewerKernelModelNodePtr{new OS9ViewerKernelModelNode{nullptr, "System globals not initialized", ""}};

    OS9::SystemGlobals globals = *kernel->m_systemGlobals;

    OS9ViewerKernelModelNodePtr rootNode{new OS9ViewerKernelModelNode{nullptr, "System globals", ""}};
    rootNode->AddItem("D_ID", toHex(globals.D_ID.Read()));
    OS9ViewerKernelModelNode* modDirNode = rootNode->AddContainer("D_ModDir");

    ssize_t modCount = globals.D_ModDirEnd - globals.D_ModDir;
    for(ssize_t i = 0; i < modCount; i++)
    {
        OS9ViewerKernelModelNode* arrayNode = modDirNode->AddContainer(std::format("[{}]", i));
        OS9ViewerKernelModelNode* moduleNode = arrayNode->AddContainer("MD_MPtr");
        OS9::ModuleDirectoryEntry modDir = globals.D_ModDir[i];

        arrayNode->AddItem("MD_Group", toHex(modDir.MD_Group.Read(), true));
        arrayNode->AddItem("MD_Static", toHex(modDir.MD_Static.Read(), true));
        arrayNode->AddItem("MD_Link", std::to_string(modDir.MD_Link.Read()));
        arrayNode->AddItem("MD_MChk", toHex(modDir.MD_MChk.Read()));

        OS9::Module module = *modDir.MD_MPtr;
        // const uint8_t* pointer = m_kernel.m_memory.GetPointer(modDir.MD_MPtr.TargetAddress());
        moduleNode->AddItem("M$ID", toHex(module.M_ID.Read()));
        moduleNode->AddItem("M$SysRev", std::to_string(module.M_SysRev.Read()));
        moduleNode->AddItem("M$Size", toHex(module.M_Size.Read(), true)); // std::to_string
        moduleNode->AddItem("M$Owner", toHex(module.M_Owner.Read()));
        const uint32_t nameOffset = module.M_Name.Read();
        const OS9::CString name{kernel->m_memory, module.Address() + nameOffset};
        arrayNode->m_name = name;
        moduleNode->AddItem("M$Name", std::to_string(module.M_Name.Read()));
        moduleNode->AddItem("M$Accs", toHex(module.M_Accs.Read(), true));
        moduleNode->AddItem("M$Type", std::to_string(module.M_Type.Read()));
        moduleNode->AddItem("M$Lang", std::to_string(module.M_Lang.Read()));
        moduleNode->AddItem("M$Attr", toHex(module.M_Attr.Read(), true));
        moduleNode->AddItem("M$Revs", std::to_string(module.M_Revs.Read()));
        moduleNode->AddItem("M$Edit", std::to_string(module.M_Edit.Read()));
        moduleNode->AddItem("M$Usage", toHex(module.M_Usage.Read(), true));
        moduleNode->AddItem("M$Symbol", toHex(module.M_Symbol.Read(), true));
        moduleNode->AddItem("M$Parity", toHex(module.M_Parity.Read()));
    }

    return rootNode;
}

OS9ViewerKernelModel::OS9ViewerKernelModel(CeDImu& cedimu)
    : m_cedimu{cedimu}
    , m_root{BuildRootNode()}
{
}

OS9ViewerKernelModel::~OS9ViewerKernelModel()
{
}

bool OS9ViewerKernelModel::IsContainer(const wxDataViewItem& item) const
{
    if(!item.IsOk())
        return true; // Root item is a container.

    const OS9ViewerKernelModelNode* node = ITEM_ID_TO_NODE(item);
    return node->IsContainer();
}

bool OS9ViewerKernelModel::HasContainerColumns(const wxDataViewItem&) const
{
    return true;
}

void OS9ViewerKernelModel::GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const
{
    if(!item.IsOk())
    {
        variant = "Invalid";
        return;
    }

    OS9ViewerKernelModelNode* node = ITEM_ID_TO_NODE(item);
    switch(col)
    {
    case ColumnMember:
        variant = wxAny(node->m_member.c_str());
        break;

    case ColumnValue:
        variant = wxAny(node->m_value.c_str());
        break;

    case ColumnName:
        variant = wxAny(node->m_name.c_str());
        break;

    default:
        wxASSERT_MSG(false, "Invalid column in GetValue");
    }
}

bool OS9ViewerKernelModel::SetValue(const wxVariant&, const wxDataViewItem&, unsigned int)
{
    return false;
}

wxDataViewItem OS9ViewerKernelModel::GetParent(const wxDataViewItem& item) const
{
    if(!item.IsOk())
        return wxDataViewItem{nullptr};

    OS9ViewerKernelModelNode* node = ITEM_ID_TO_NODE(item);
    return wxDataViewItem{node->GetParent()};
}

unsigned int OS9ViewerKernelModel::GetChildren(const wxDataViewItem& item, wxDataViewItemArray& array) const
{
    if(!item.IsOk())
    {
        array.Add(wxDataViewItem{m_root.get()});
        return 1;
    }

    OS9ViewerKernelModelNode* node = ITEM_ID_TO_NODE(item);
    const OS9ViewerKernelModelNodePtrVector& children = node->GetChildren();
    for(const OS9ViewerKernelModelNodePtr& node : children)
        array.Add(wxDataViewItem{node.get()});

    return children.size();
}
