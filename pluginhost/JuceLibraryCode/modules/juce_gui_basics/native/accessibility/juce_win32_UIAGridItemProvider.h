/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
class UIAGridItemProvider  : public UIAProviderBase,
                             public ComBaseClassHelper<ComTypes::IGridItemProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    JUCE_COMRESULT get_Row (int* pRetVal) override
    {
        return withCellInterface (pRetVal, [&] (const AccessibilityCellInterface& cellInterface)
        {
            *pRetVal = cellInterface.getRowIndex();
        });
    }

    JUCE_COMRESULT get_Column (int* pRetVal) override
    {
        return withCellInterface (pRetVal, [&] (const AccessibilityCellInterface& cellInterface)
        {
            *pRetVal = cellInterface.getColumnIndex();
        });
    }

    JUCE_COMRESULT get_RowSpan (int* pRetVal) override
    {
        return withCellInterface (pRetVal, [&] (const AccessibilityCellInterface& cellInterface)
        {
            *pRetVal = cellInterface.getRowSpan();
        });
    }

    JUCE_COMRESULT get_ColumnSpan (int* pRetVal) override
    {
        return withCellInterface (pRetVal, [&] (const AccessibilityCellInterface& cellInterface)
        {
            *pRetVal = cellInterface.getColumnSpan();
        });
    }

    JUCE_COMRESULT get_ContainingGrid (IRawElementProviderSimple** pRetVal) override
    {
        return withCellInterface (pRetVal, [&] (const AccessibilityCellInterface& cellInterface)
        {
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

            if (auto* handler = cellInterface.getTableHandler())
                handler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        });
    }

private:
    template <typename Value, typename Callback>
    JUCE_COMRESULT withCellInterface (Value* pRetVal, Callback&& callback) const
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* cellInterface = getHandler().getCellInterface())
            {
                callback (*cellInterface);
                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAGridItemProvider)
};

} // namespace juce
