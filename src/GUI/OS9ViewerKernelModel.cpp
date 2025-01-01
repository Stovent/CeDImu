#include "OS9ViewerKernelModel.hpp"
#include "../CDI/common/utils.hpp"

#define ITEM_ID_TO_NODE(item) (static_cast<OS9ViewerKernelModelNode*>(item.GetID()))

OS9ViewerKernelModelNode::OS9ViewerKernelModelNode(OS9ViewerKernelModelNode* parent, const std::string& containerName)
    : m_member{containerName}
    , m_value{}
    , m_isContainer{true}
    , m_parent{parent}
{}

OS9ViewerKernelModelNode::OS9ViewerKernelModelNode(OS9ViewerKernelModelNode* parent, const std::string& itemName, const std::string& itemValue)
    : m_member{itemName}
    , m_value{itemValue}
    , m_isContainer{false}
    , m_parent{parent}
{}

template<typename... Args>
OS9ViewerKernelModelNode* OS9ViewerKernelModelNode::AddChildren(Args&&... args)
{
    m_children.emplace_back(std::make_unique<OS9ViewerKernelModelNode, OS9ViewerKernelModelNode*, Args...>(this, std::forward<Args>(args)...));
    return m_children.rbegin()->get();
}

OS9ViewerKernelModelNodePtr OS9ViewerKernelModel::BuildRootNode()
{
    OS9::Pointer<OS9::SystemGlobals> globals = m_kernel.m_systemGlobals;
    OS9ViewerKernelModelNodePtr rootNode{new OS9ViewerKernelModelNode{nullptr, "System globals"}};
    rootNode->AddChildren("D_ID", toHex(globals->D_ID.Read()));
    OS9ViewerKernelModelNode* modDirNode = rootNode->AddChildren("D_ModDir");

    ssize_t modCount = globals->D_ModDirEnd - globals->D_ModDir;
    for(ssize_t i = 0; i < modCount; i++)
    {
        OS9ViewerKernelModelNode* arrayNode = modDirNode->AddChildren(std::format("[{}]", i));
        OS9ViewerKernelModelNode* moduleNode = arrayNode->AddChildren("MD_MPtr");
        OS9::ModuleDirectoryEntry modDir = globals->D_ModDir[i];

        arrayNode->AddChildren("MD_Group", toHex(modDir.MD_Group.Read(), true));
        arrayNode->AddChildren("MD_Static", toHex(modDir.MD_Static.Read(), true));
        arrayNode->AddChildren("MD_Link", std::to_string(modDir.MD_Link.Read()));
        arrayNode->AddChildren("MD_MChk", toHex(modDir.MD_MChk.Read()));

        OS9::Module module = *modDir.MD_MPtr;
        const uint8_t* pointer = m_kernel.m_memory.GetPointer(modDir.MD_MPtr.TargetAddress());
        moduleNode->AddChildren("M$ID", toHex(module.M_ID.Read()));
        moduleNode->AddChildren("M$SysRev", std::to_string(module.M_SysRev.Read()));
        moduleNode->AddChildren("M$Size", toHex(module.M_Size.Read(), true)); // std::to_string
        moduleNode->AddChildren("M$Owner", toHex(module.M_Owner.Read()));
        const uint32_t nameOffset = module.M_Name.Read();
        moduleNode->AddChildren("M$Name", reinterpret_cast<const char*>(pointer + nameOffset));
        moduleNode->AddChildren("M$Accs", toHex(module.M_Accs.Read(), true));
        moduleNode->AddChildren("M$Type", std::to_string(module.M_Type.Read()));
        moduleNode->AddChildren("M$Lang", std::to_string(module.M_Lang.Read()));
        moduleNode->AddChildren("M$Attr", toHex(module.M_Attr.Read(), true));
        moduleNode->AddChildren("M$Revs", std::to_string(module.M_Revs.Read()));
        moduleNode->AddChildren("M$Edit", std::to_string(module.M_Edit.Read()));
        moduleNode->AddChildren("M$Usage", toHex(module.M_Usage.Read(), true));
        moduleNode->AddChildren("M$Symbol", toHex(module.M_Symbol.Read(), true));
        moduleNode->AddChildren("M$Parity", toHex(module.M_Parity.Read()));
    }

    return rootNode;
}

OS9ViewerKernelModel::OS9ViewerKernelModel(CeDImu& cedimu)
    : m_cedimu{cedimu}
    , m_kernel{{std::bind(&CeDImu::GetByte, &m_cedimu, std::placeholders::_1),
                std::bind(&CeDImu::GetWord, &m_cedimu, std::placeholders::_1),
                std::bind(&CeDImu::GetPointer, &m_cedimu, std::placeholders::_1)}}
    , m_root{BuildRootNode()}
{
}

OS9ViewerKernelModel::~OS9ViewerKernelModel()
{
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

bool OS9ViewerKernelModel::IsContainer(const wxDataViewItem& item) const
{
    if(!item.IsOk())
        return true; // Root item is a container.

    const OS9ViewerKernelModelNode* node = ITEM_ID_TO_NODE(item);
    return node->IsContainer();
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
