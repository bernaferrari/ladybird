/*
 * Copyright (c) 2025, the Ladybird Browser contributors
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Layout/MathMLActionBox.h>
#include <LibWeb/MathML/MathMLActionElement.h>

namespace Web::MathML {

GC_DEFINE_ALLOCATOR(MathMLActionElement);

MathMLActionElement::MathMLActionElement(DOM::Document& document, DOM::QualifiedName qualified_name)
    : MathMLElement(document, move(qualified_name))
{
}

GC::Ptr<Layout::Node> MathMLActionElement::create_layout_node(GC::Ref<CSS::ComputedProperties> style)
{
    return heap().allocate<Layout::MathMLActionBox>(document(), *this, move(style));
}

}
