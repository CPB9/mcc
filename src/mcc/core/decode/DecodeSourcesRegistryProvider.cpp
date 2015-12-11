/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <fstream>

#include "pegtl.hh"

#include "mcc/core/decode/DecodeSourcesRegistryProvider.h"

namespace mcc
{
namespace decode
{
    
/**! Парсер */

using namespace pegtl;

struct TrueK : string<'T', 'r', 'u', 'e'>{};
struct FalseK : string<'F', 'a', 'l', 's', 'e'>{};
struct NamespaceK : string<'n', 'a', 'm', 'e', 's', 'p', 'a', 'c', 'e'>{};
struct InfoK : string<'i', 'n', 'f', 'o'>{};
struct EnumK : string<'e', 'n', 'u', 'm'>{};
struct StructK : string<'s', 't', 'r', 'u', 'c', 't'>{};
struct CommandK : string<'c', 'o', 'm', 'm', 'a', 'n', 'd'>{};
struct StatusK : string<'s', 't', 'a', 't', 'u', 's'>{};
struct MessageK : string<'m', 'e', 's', 's', 'a', 'g', 'e'>{};
struct ComponentK : string<'c', 'o', 'm', 'p', 'o', 'n', 'e', 'n', 't'>{};
struct UnitK : string<'u', 'n', 'i', 't'>{};
struct DisplayK : string<'d', 'i', 's', 'p', 'l', 'a', 'y'>{};
struct FloatK : string<'f', 'l', 'o', 'a', 't'>{};
struct UintK : string<'u', 'i', 'n', 't'>{};
struct IntK : string<'i', 'n', 't'>{};
struct EventK : string<'e', 'v', 'e', 'n', 't'>{};
struct DynamicK : string<'d', 'y', 'n', 'a', 'm', 'i', 'c'>{};
struct BoolK : string<'b', 'o', 'o', 'l'>{};
struct AliasK : string<'a', 'l', 'i', 'a', 's'>{};
struct TypeK : string<'t', 'y', 'p', 'e'>{};

struct DotS : one<'.'>{};
struct MinusS : one<'-'>{};
struct PlusS : one<'+'>{};
struct SmallES : one<'e'>{};
struct BigES : one<'E'>{};
struct LeftParenS : one<'('>{};
struct RightParenS : one<')'>{};
struct SlashS : one<'/'>{};
struct CommaS : one<','>{};
struct EqS : one<'='>{};
struct StarS : one<'*'>{};
struct ColonS : one<':'>{};
struct LeftBracketS : one<'['>{};
struct RightBracketS : one<']'>{};
struct LeftBraceS : one<'{'>{};
struct RightBraceS : one<'}'>{};

struct TypeApplicationR;

struct EwR : plus<space>{};
struct OewR : star<space>{};
struct StringValueR : sor<seq<one<'\''>, star<not_one<'\''>>, one<'\''>>, seq<one<'"'>, star<not_one<'"'>>, one<'"'>>>{};
struct ElementNameR : identifier{};
struct ElementFqnR : seq<ElementNameR, star<DotS, ElementNameR>>{};
struct NamespaceR : seq<NamespaceK, EwR, ElementFqnR>{};
struct InfoR : seq<InfoK, EwR, StringValueR>{};
struct OptInfoR : opt<OewR, InfoR>{};
struct NonNegativeIntR : star<digit>{};
struct FloatLiteralR : seq<opt<sor<MinusS, PlusS>>,
    sor<seq<NonNegativeIntR, DotS, opt<NonNegativeIntR>>, seq<DotS, NonNegativeIntR>>,
    opt<sor<SmallES, BigES>, opt<sor<PlusS, MinusS>>, NonNegativeIntR>>{};
struct LiteralR : sor<FloatLiteralR, NonNegativeIntR, TrueK, FalseK>{};
struct EnumValueR : seq<ElementNameR, OewR, EqS, OewR, LiteralR, OptInfoR>{};
struct EnumValuesR : seq<EnumValueR, star<OewR, CommaS, OewR, EnumValueR>, opt<OewR, CommaS>>{};
struct EnumR : seq<EnumK, EwR, ElementFqnR, OptInfoR, OewR, LeftParenS, OewR, EnumValuesR, OewR, RightParenS>{};
struct UnitApplicationR : seq<SlashS,OewR, ElementFqnR, OewR, SlashS>{};
struct TypeUnitApplicatonR : seq<TypeApplicationR, opt<EwR, UnitApplicationR>>{};
struct CommandArgR : seq<TypeUnitApplicatonR, EwR, ElementNameR, OptInfoR>{};
struct CommandArgsR : seq<CommandArgR, star<OewR, CommaS, OewR, CommandArgR>, opt<OewR, CommaS>>{};
struct StructR : seq<StructK, OptInfoR, OewR, LeftParenS, OewR, CommandArgsR, OewR, RightParenS>{};
struct TypeBodyR : sor<EnumR, StructR, seq<TypeApplicationR, OptInfoR>>{};
struct ComponentBaseTypeR : TypeBodyR{};
struct CommandR : seq<CommandK, EwR, ElementNameR, OewR, ColonS, OewR, NonNegativeIntR, OptInfoR, OewR, LeftParenS, opt<OewR, CommandArgsR>, OewR, RightParenS>{};

struct DeepAllParametersR : seq<StarS, DotS, StarS>{};
struct AllParametersR : StarS{};
struct DotsR : seq<DotS, DotS>{};
struct ParameterElementR : seq<ElementFqnR, opt<
    plus<LeftBracketS, NonNegativeIntR, opt<DotsR, OewR, NonNegativeIntR>, RightBracketS>,
    star<DotS, ElementFqnR, opt<LeftBracketS, NonNegativeIntR, opt<DotsR, NonNegativeIntR>, RightBracketS>>>>{};
struct ParameterR : sor<AllParametersR, seq<ParameterElementR, OptInfoR>>{};
struct MessageParametersR : sor<DeepAllParametersR, AllParametersR, seq<LeftParenS, OewR, ParameterR, star<OewR, CommaS, OewR, ParameterR>, opt<OewR, CommaS>, OewR, RightParenS>>{};
struct StatusMessageR : seq<StatusK, EwR, MessageParametersR>{};
struct EventMessageR : seq<EventK, EwR, MessageParametersR>{};
struct DynamicStatusMessageR : seq<DynamicK, EwR, StatusK, EwR, MessageParametersR>{};
struct MessageR : seq<MessageK, OewR, ElementNameR, OewR, ColonS, OewR, NonNegativeIntR, OptInfoR, EwR, sor<StatusMessageR, EventMessageR, DynamicStatusMessageR>>{};
struct SubComponentR : ElementFqnR{};
struct ComponentR : seq<
    ComponentK, EwR, ElementNameR,
    opt<OewR, ColonS, OewR, SubComponentR, star<OewR, CommaS, OewR, SubComponentR>>,
    OptInfoR,
    OewR, LeftBraceS, opt<OewR, not_at<sor<CommandR, MessageR>>, ComponentBaseTypeR>, star<OewR, sor<CommandR, MessageR>>, OewR, RightBraceS>{};
struct UnitR : seq<UnitK, EwR, ElementNameR, EwR, DisplayK, EwR, StringValueR, EwR, InfoR>{};
struct TypeR : seq<TypeK, EwR, ElementNameR, OptInfoR, EwR, TypeBodyR>{};
struct PrimitiveTypeKindR : sor<FloatK, UintK, IntK, BoolK>{};
struct PrimitiveTypeApplicationR : seq<PrimitiveTypeKindR, ColonS, NonNegativeIntR>{};
struct ArrayTypeApplicationR : seq<LeftBracketS, OewR, TypeApplicationR, OewR, CommaS, OewR, NonNegativeIntR, opt<OewR, DotS, DotS, OewR, NonNegativeIntR>, OewR, RightBracketS>{};
struct TypeApplicationR : sor<PrimitiveTypeApplicationR, ArrayTypeApplicationR, ElementFqnR>{};
struct AliasR : seq<AliasK, EwR, ElementNameR, EwR, TypeApplicationR, opt<EwR, InfoR>>{};
struct IfDevParser : must<OewR, NamespaceR, star<OewR, sor<ComponentR, UnitR, TypeR, AliasR>>, OewR, eof>{};

template<typename Rule>
struct tracer : normal<Rule>
{
    template< typename Input, typename ... States >
    static void start( const Input & in, States && ... )
    {
        std::cerr << in.data().line << ":" << in.data().column << "  start  " << internal::demangle< Rule >() << std::endl;
    }

    template< typename Input, typename ... States >
    static void success( const Input & in, States && ... )
    {
        std::cerr << in.data().line << ":" << in.data().column << " success " << internal::demangle< Rule >() << std::endl;
    }

    template< typename Input, typename ... States >
    static void failure( const Input & in, States && ... )
    {
        std::cerr << in.data().line << ":" << in.data().column << " failure " << internal::demangle< Rule >() << std::endl;
    }
};

template< typename Rule, template< typename ... > class Action = nothing, typename Input, typename ... States >
bool trace_input( Input & in, States && ... st )
{
    return parse_input<Rule, Action, tracer>( in, st ... );
}

template<typename Rule, template<typename...> class Action = nothing, typename... Args>
bool trace(Args && ... args)
{
    return parse<Rule, Action, tracer>(std::forward<Args>(args)...);
}

    
std::unique_ptr<Registry> DecodeSourcesRegistryProvider::provide()
{
    std::unique_ptr<Registry> registry(new Registry(std::vector<std::shared_ptr<Namespace>>()));
    for (auto& source : _sources)
    {
        std::ifstream in(source, std::ios::in | std::ios::binary);
        BMCL_ASSERT_MSG(in, "source not found");
        std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        // try 'trace' on error
        if (!parse<IfDevParser>(text, text))
        {
            bmcl::panic("parse error");
        }
    }
    return registry;
}
}
}
