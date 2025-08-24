#ifndef GUI_OS9VIEWERKERNELMODEL_HPP
#define GUI_OS9VIEWERKERNELMODEL_HPP

#include "../CeDImu.hpp"
#include "../CDI/OS9/Kernel.hpp"

#include <wx/dataview.h>

#include <memory>
#include <string>
#include <vector>

class OS9ViewerKernelModelNode;
using OS9ViewerKernelModelNodePtr = std::unique_ptr<OS9ViewerKernelModelNode>;
using OS9ViewerKernelModelNodePtrVector = std::vector<OS9ViewerKernelModelNodePtr>;

class OS9ViewerKernelModelNode
{
public:
    std::string m_member;
    std::string m_value;
    std::string m_name;

    /** \brief Constructs a container. */
    OS9ViewerKernelModelNode(OS9ViewerKernelModelNode* parent, const std::string& containerName, const std::string& displayName);
    /** \brief Constructs an item. */
    OS9ViewerKernelModelNode(OS9ViewerKernelModelNode* parent, const std::string& itemName, const std::string& itemValue, const std::string& displayName);

    bool IsContainer() const { return m_isContainer; }

    OS9ViewerKernelModelNode* AddContainer(const std::string& containerName, const std::string& displayName = "");
    OS9ViewerKernelModelNode* AddItem(const std::string& itemName, const std::string& itemValue, const std::string& displayName = "");

    const OS9ViewerKernelModelNodePtrVector& GetChildren() const noexcept { return m_children; }
    OS9ViewerKernelModelNodePtrVector& GetChildren() noexcept { return m_children; }

    OS9ViewerKernelModelNode* GetParent() const noexcept { return m_parent; }

private:
    bool m_isContainer;
    OS9ViewerKernelModelNode* m_parent;
    OS9ViewerKernelModelNodePtrVector m_children;

    template<typename... Args>
    OS9ViewerKernelModelNode* AddChildren(Args&&... args);
};

class OS9ViewerKernelModel : public wxDataViewModel
{
public:
    CeDImu& m_cedimu;

    OS9ViewerKernelModelNodePtr m_root;

    enum : unsigned int
    {
        ColumnMember,
        ColumnValue,
        ColumnName,
    };

    OS9ViewerKernelModel(CeDImu& cedimu);
    ~OS9ViewerKernelModel();

    OS9ViewerKernelModelNodePtr BuildRootNode();

    virtual bool IsContainer(const wxDataViewItem& item) const override;
    virtual bool HasContainerColumns(const wxDataViewItem& item) const override;

    virtual void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const override;
    virtual bool SetValue(const wxVariant&, const wxDataViewItem&, unsigned int) override;

    virtual wxDataViewItem GetParent(const wxDataViewItem& item) const override;
    virtual unsigned int GetChildren(const wxDataViewItem& item, wxDataViewItemArray& array) const override;
};

#endif // GUI_OS9VIEWERKERNELMODEL_HPP
