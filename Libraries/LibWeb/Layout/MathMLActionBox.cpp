/*
 * Copyright (c) 2025, the Ladybird Browser contributors
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Layout/MathMLActionBox.h>
#include <LibWeb/MathML/MathMLActionElement.h>

namespace Web::Layout {

MathMLActionBox::MathMLActionBox(DOM::Document& document, MathML::MathMLActionElement& element, GC::Ref<CSS::ComputedProperties> style)
    : MathMLBox(document, element, move(style))
{
}

}
