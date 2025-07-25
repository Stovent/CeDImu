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

    /** \brief Constructs a container. */
    OS9ViewerKernelModelNode(OS9ViewerKernelModelNode* parent, const std::string& containerName);
    /** \brief Constructs an item. */
    OS9ViewerKernelModelNode(OS9ViewerKernelModelNode* parent, const std::string& itemName, const std::string& itemValue);

    bool IsContainer() const { return m_isContainer; }

    /** \brief Contructs in this node the child (without the parent pointer which is automatically added).
     * \return The newly constructed child.
     */
    template<typename... Args>
    OS9ViewerKernelModelNode* AddChildren(Args&&... args);
    const OS9ViewerKernelModelNodePtrVector& GetChildren() const noexcept { return m_children; }
    OS9ViewerKernelModelNodePtrVector& GetChildren() noexcept { return m_children; }

    OS9ViewerKernelModelNode* GetParent() const noexcept { return m_parent; }

private:
    bool m_isContainer;
    OS9ViewerKernelModelNode* m_parent;
    OS9ViewerKernelModelNodePtrVector m_children;
};

class OS9ViewerKernelModel : public wxDataViewModel
{
public:
    CeDImu& m_cedimu;
    OS9::EmulatedMemoryAccess m_emulatedMemoryAccess;
    OS9::Pointer<OS9::SystemGlobals> m_systemGlobalsPtr;
    // OS9::Kernel m_kernel;

    OS9ViewerKernelModelNodePtr m_root;

    enum : unsigned int
    {
        ColumnMember,
        ColumnValue,
        // ColumnHexValue,
    };

    OS9ViewerKernelModel() = delete;
    OS9ViewerKernelModel(CeDImu& cedimu);
    ~OS9ViewerKernelModel();

    OS9ViewerKernelModelNodePtr BuildRootNode();
    // void BuildRootNode();

    virtual void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const override;
    virtual bool SetValue(const wxVariant&, const wxDataViewItem&, unsigned int) override;
    virtual wxDataViewItem GetParent(const wxDataViewItem& item) const override;
    virtual bool IsContainer(const wxDataViewItem& item) const override;
    virtual unsigned int GetChildren(const wxDataViewItem& item, wxDataViewItemArray& array) const override;
};

#endif // GUI_OS9VIEWERKERNELMODEL_HPP
