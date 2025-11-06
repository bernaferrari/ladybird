/*
 * Copyright (c) 2025, the Ladybird Browser contributors
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/Layout/MathMLBox.h>
#include <LibWeb/MathML/MathMLActionElement.h>

namespace Web::Layout {

class MathMLActionBox final : public MathMLBox {
    GC_CELL(MathMLActionBox, MathMLBox);

public:
    MathMLActionBox(DOM::Document&, MathML::MathMLActionElement&, GC::Ref<CSS::ComputedProperties>);
    virtual ~MathMLActionBox() override = default;

    MathML::MathMLActionElement& dom_node() { return static_cast<MathML::MathMLActionElement&>(MathMLBox::dom_node()); }
    MathML::MathMLActionElement const& dom_node() const { return static_cast<MathML::MathMLActionElement const&>(MathMLBox::dom_node()); }

private:
    virtual bool is_mathml_action_box() const final { return true; }
};

template<>
inline bool Node::fast_is<MathMLActionBox>() const { return is_mathml_action_box(); }

}
