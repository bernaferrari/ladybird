/*
 * Copyright (c) 2025, the Ladybird Browser contributors
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/ARIA/Roles.h>
#include <LibWeb/DOM/Element.h>
#include <LibWeb/MathML/MathMLElement.h>

namespace Web::MathML {

class MathMLActionElement final : public MathMLElement {
    WEB_PLATFORM_OBJECT(MathMLActionElement, MathMLElement);
    GC_DECLARE_ALLOCATOR(MathMLActionElement);

public:
    virtual ~MathMLActionElement() override = default;

    virtual void initialize(JS::Realm&) override;
    virtual GC::Ptr<Layout::Node> create_layout_node(GC::Ref<CSS::ComputedProperties>) override;
    virtual Optional<ARIA::Role> default_role() const override;
    virtual i32 default_tab_index_value() const override;
    virtual bool is_focusable() const override;
    bool supports_tooltip() const;
    Optional<String> tooltip_text() const;
    bool is_child_selected(DOM::Node const&) const;
    virtual void attribute_changed(FlyString const& local_name, Optional<String> const& old_value, Optional<String> const& value, Optional<FlyString> const& namespace_) override;
    virtual void children_changed(DOM::Node::ChildrenChangedMetadata const*) override;
    virtual void inserted() override;

private:
    enum class ActionType {
        Toggle,
        Tooltip,
        Statusline,
        Unknown,
    };

    MathMLActionElement(DOM::Document&, DOM::QualifiedName);

    void update_selection_from_attribute();
    void update_selected_child();
    DOM::Element* selectable_child_at(size_t index) const;
    DOM::Element* first_selectable_child() const;
    void invalidate_children_styles();
    void update_action_type_from_attribute();
    void handle_activation(DOM::Event&);
    bool advance_selection();
    size_t selectable_child_count() const;
    size_t index_of_selectable_child(DOM::Element const&) const;
    void set_selection_attribute(size_t index);
    Optional<String> metadata_text_from_child(size_t index) const;
    virtual void visit_edges(JS::Cell::Visitor&) override;

    size_t m_selection_index { 1 };
    bool m_selection_is_explicit { false };
    DOM::Element* m_selected_child { nullptr };
    ActionType m_action_type { ActionType::Toggle };
    GC::Ptr<DOM::IDLEventListener> m_activation_event_listener;
};

}
