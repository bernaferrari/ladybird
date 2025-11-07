/*
 * Copyright (c) 2025, the Ladybird Browser contributors
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/String.h>
#include <AK/StringView.h>
#include <AK/TypeCasts.h>
#include <LibJS/Runtime/NativeFunction.h>
#include <LibWeb/DOM/Element.h>
#include <LibWeb/DOM/Event.h>
#include <LibWeb/DOM/IDLEventListener.h>
#include <LibWeb/HTML/AttributeNames.h>
#include <LibWeb/Layout/MathMLActionBox.h>
#include <LibWeb/MathML/AttributeNames.h>
#include <LibWeb/MathML/MathMLActionElement.h>
#include <LibWeb/UIEvents/EventNames.h>
#include <LibWeb/UIEvents/KeyboardEvent.h>

namespace Web::MathML {

static constexpr bool mathml_action_debug = false;

GC_DEFINE_ALLOCATOR(MathMLActionElement);

MathMLActionElement::MathMLActionElement(DOM::Document& document, DOM::QualifiedName qualified_name)
    : MathMLElement(document, move(qualified_name))
{
}

void MathMLActionElement::initialize(JS::Realm& realm)
{
    Base::initialize(realm);

    auto activation_callback_function = JS::NativeFunction::create(
        realm, [this](JS::VM& vm) {
            if (vm.argument_count() == 0 || !vm.argument(0).is_object())
                return JS::js_undefined();
            auto& object = vm.argument(0).as_object();
            if (!is<DOM::Event>(object))
                return JS::js_undefined();
            auto& event = static_cast<DOM::Event&>(object);
            handle_activation(event);
            return JS::js_undefined();
        },
        0, Utf16FlyString {}, &realm);
    auto activation_callback = realm.heap().allocate<WebIDL::CallbackType>(*activation_callback_function, realm);

    m_activation_event_listener = DOM::IDLEventListener::create(realm, activation_callback);
    add_event_listener_without_options(UIEvents::EventNames::click, *m_activation_event_listener);
    add_event_listener_without_options(UIEvents::EventNames::keydown, *m_activation_event_listener);
}

GC::Ptr<Layout::Node> MathMLActionElement::create_layout_node(GC::Ref<CSS::ComputedProperties> style)
{
    update_selected_child();
    return heap().allocate<Layout::MathMLActionBox>(document(), *this, move(style));
}

Optional<ARIA::Role> MathMLActionElement::default_role() const
{
    if (m_action_type == ActionType::Toggle)
        return ARIA::Role::button;
    return MathMLElement::default_role();
}

i32 MathMLActionElement::default_tab_index_value() const
{
    if (m_action_type == ActionType::Toggle)
        return 0;
    return DOM::Element::default_tab_index_value();
}

bool MathMLActionElement::is_focusable() const
{
    if (get_attribute(HTML::AttributeNames::tabindex).has_value())
        return true;
    return m_action_type == ActionType::Toggle;
}

bool MathMLActionElement::supports_tooltip() const
{
    return m_action_type == ActionType::Tooltip || m_action_type == ActionType::Statusline;
}

Optional<String> MathMLActionElement::tooltip_text() const
{
    if (!supports_tooltip())
        return {};
    return metadata_text_from_child(2);
}

bool MathMLActionElement::is_child_selected(DOM::Node const& child) const
{
    if (!m_selected_child)
        return false;
    if (!is<DOM::Element>(child))
        return false;
    auto const& element = static_cast<DOM::Element const&>(child);
    return &element == m_selected_child;
}

void MathMLActionElement::attribute_changed(FlyString const& local_name, Optional<String> const& old_value, Optional<String> const& value, Optional<FlyString> const& namespace_)
{
    MathMLElement::attribute_changed(local_name, old_value, value, namespace_);
    if (local_name == AttributeNames::selection) {
        update_selection_from_attribute();
        return;
    }
    if (local_name == AttributeNames::actiontype) {
        update_action_type_from_attribute();
        return;
    }
}

void MathMLActionElement::children_changed(DOM::Node::ChildrenChangedMetadata const* metadata)
{
    MathMLElement::children_changed(metadata);
    update_selected_child();
}

void MathMLActionElement::inserted()
{
    MathMLElement::inserted();
    update_selection_from_attribute();
    update_action_type_from_attribute();
}

void MathMLActionElement::update_selection_from_attribute()
{
    size_t new_index = 1;
    bool new_selection_is_explicit = false;
    if (auto selection_attribute = attribute(AttributeNames::selection); selection_attribute.has_value()) {
        if (auto maybe_number = selection_attribute->to_number<int>(); maybe_number.has_value() && maybe_number.value() > 0) {
            new_index = static_cast<size_t>(maybe_number.value());
            new_selection_is_explicit = true;
        }
    }
    if (new_index == m_selection_index && new_selection_is_explicit == m_selection_is_explicit && m_selected_child)
        return;
    m_selection_index = new_index;
    m_selection_is_explicit = new_selection_is_explicit;
    update_selected_child();
}

void MathMLActionElement::update_selected_child()
{
    auto* candidate = selectable_child_at(m_selection_index);
    if (!candidate && !m_selection_is_explicit)
        candidate = first_selectable_child();

    if (candidate == m_selected_child)
        return;

    m_selected_child = candidate;
    dbgln_if(mathml_action_debug, "<maction> selected child -> {} ({})", m_selected_child ? m_selected_child->debug_description() : StringView("(none)"sv), static_cast<void*>(m_selected_child));
    invalidate_children_styles();
}

DOM::Element* MathMLActionElement::selectable_child_at(size_t index) const
{
    if (index == 0)
        return nullptr;

    size_t current_index = 0;
    for (auto const* child = first_child(); child; child = child->next_sibling()) {
        if (!is<DOM::Element>(*child))
            continue;
        ++current_index;
        if (current_index == index)
            return const_cast<DOM::Element*>(static_cast<DOM::Element const*>(child));
    }
    return nullptr;
}

DOM::Element* MathMLActionElement::first_selectable_child() const
{
    for (auto const* child = first_child(); child; child = child->next_sibling()) {
        if (is<DOM::Element>(*child))
            return const_cast<DOM::Element*>(static_cast<DOM::Element const*>(child));
    }
    return nullptr;
}

void MathMLActionElement::invalidate_children_styles()
{
    for (auto* child = first_child(); child; child = child->next_sibling()) {
        if (!is<DOM::Element>(*child))
            continue;
        static_cast<DOM::Element&>(*child).invalidate_style(DOM::StyleInvalidationReason::Other);
    }
    set_needs_layout_tree_update(true, DOM::SetNeedsLayoutTreeUpdateReason::StyleChange);
    set_child_needs_layout_tree_update(true);
}

void MathMLActionElement::update_action_type_from_attribute()
{
    auto type = attribute(AttributeNames::actiontype).value_or(String {}); // empty string -> default toggle

    if (type.is_empty() || type.equals_ignoring_ascii_case("toggle"sv))
        m_action_type = ActionType::Toggle;
    else if (type.equals_ignoring_ascii_case("tooltip"sv))
        m_action_type = ActionType::Tooltip;
    else if (type.equals_ignoring_ascii_case("statusline"sv))
        m_action_type = ActionType::Statusline;
    else
        m_action_type = ActionType::Toggle; // Unknown values fall back to toggle per MathML Core.
}

void MathMLActionElement::handle_activation(DOM::Event& event)
{
    if (m_action_type != ActionType::Toggle)
        return;

    if (event.type() == UIEvents::EventNames::keydown) {
        if (!is<UIEvents::KeyboardEvent>(event))
            return;
        auto const& keyboard_event = static_cast<UIEvents::KeyboardEvent const&>(event);
        auto key = keyboard_event.key();
        if (!(key == " "sv || key.equals_ignoring_ascii_case("Enter"sv)))
            return;
    } else if (event.type() != UIEvents::EventNames::click) {
        return;
    }

    if (!advance_selection())
        return;

    event.prevent_default();
}

bool MathMLActionElement::advance_selection()
{
    auto count = selectable_child_count();
    if (count == 0)
        return false;

    size_t current_index = m_selected_child ? index_of_selectable_child(*m_selected_child) : 0;
    size_t next_index = current_index == 0 ? 1 : (current_index % count) + 1;

    if (next_index == current_index && count == 1)
        return false;

    set_selection_attribute(next_index);
    return true;
}

size_t MathMLActionElement::selectable_child_count() const
{
    size_t count = 0;
    for (auto const* child = first_child(); child; child = child->next_sibling()) {
        if (is<DOM::Element>(*child))
            ++count;
    }
    return count;
}

size_t MathMLActionElement::index_of_selectable_child(DOM::Element const& needle) const
{
    size_t index = 0;
    for (auto const* child = first_child(); child; child = child->next_sibling()) {
        if (!is<DOM::Element>(*child))
            continue;
        ++index;
        if (&static_cast<DOM::Element const&>(*child) == &needle)
            return index;
    }
    return 0;
}

void MathMLActionElement::set_selection_attribute(size_t index)
{
    if (index == 0)
        index = 1;
    auto selection_value = String::number(index);
    MUST(set_attribute(AttributeNames::selection, selection_value));
}

void MathMLActionElement::visit_edges(JS::Cell::Visitor& visitor)
{
    MathMLElement::visit_edges(visitor);
    visitor.visit(m_activation_event_listener);
}

Optional<String> MathMLActionElement::metadata_text_from_child(size_t index) const
{
    auto* child = selectable_child_at(index);
    if (!child)
        return {};

    auto text_content = child->text_content();
    if (!text_content.has_value())
        return {};

    auto text = text_content->to_utf8();
    auto trimmed_or_error = text.trim_whitespace(TrimMode::Both);
    if (trimmed_or_error.is_error())
        return {};
    auto trimmed = trimmed_or_error.release_value();
    if (trimmed.is_empty())
        return {};
    return trimmed;
}

}
