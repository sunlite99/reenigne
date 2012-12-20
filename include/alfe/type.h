#include "alfe/main.h"

#ifndef INCLUDED_TYPE_H
#define INCLUDED_TYPE_H

#include "alfe/any.h"
#include "alfe/hash_table.h"
#include "alfe/value.h"

class Kind
{
public:
    static Kind type;
    static Kind variadic;
    static Kind variadicTemplate;
    String toString() const { return _implementation->toString(); }
    bool operator==(const Kind& other) const
    {
        if (_implementation == other._implementation)
            return true;
        return _implementation->equals(other._implementation);
    }
    bool operator!=(const Kind& other) const { return !operator==(other); }
protected:
    class Implementation : public ReferenceCounted
    {
    public:
        virtual String toString() const = 0;
        virtual bool equals(const Implementation* other) const
        {
            return false;
        }
    };
    Kind(const Implementation* implementation)
      : _implementation(implementation) { }
private:
    class TypeImplementation : public Implementation
    {
    public:
        String toString() const { return String(); }
    };
    class VariadicImplementation : public Implementation
    {
    public:
        String toString() const { return String("..."); }
    };
    ConstReference<Implementation> _implementation;
    friend class TemplateKind;
};

Kind Kind::type = Kind(new Kind::TypeImplementation);
Kind Kind::variadic = Kind(new Kind::VariadicImplementation);

class TemplateKind : public Kind
{
public:
    // Pass in firstParameterKind and the Kind of the result is
    // restParameterKind. 
    TemplateKind(const Kind& firstParameterKind, const Kind& restParameterKind)
      : Kind(new Implementation(firstParameterKind, restParameterKind)) { }
    TemplateKind(const Kind& kind) : Kind(kind) { }
    Kind first() const { return implementation()->first(); }
    Kind rest() const { return implementation()->rest(); }
private:
    class Implementation : public Kind::Implementation
    {
    public:
        Implementation(const Kind& firstParameterKind,
            const Kind& restParameterKind)
          : _firstParameterKind(firstParameterKind),
            _restParameterKind(restParameterKind) { }
        String toString() const
        {
            String s("<");
            TemplateKind k(this);
            bool needComma = false;
            do {
                if (needComma)
                    s += ", ";
                s += k.first().toString();
                k = k.rest();
                needComma = true;
            } while (k != Kind::type);
            return s + ">";
        }
        bool equals(const Kind::Implementation* other) const
        {
            const Implementation* o =
                dynamic_cast<const Implementation*>(other);
            if (o == 0)
                return false;
            return _firstParameterKind == o->_firstParameterKind &&
                _restParameterKind == o->_restParameterKind;
        }
        Kind first() const { return _firstParameterKind; }
        Kind rest() const { return _restParameterKind; }
    private:
        Kind _firstParameterKind;
        Kind _restParameterKind;
    };
    const Implementation* implementation() const
    {
        return _implementation.referent<Implementation>();
    }
};

Kind Kind::variadicTemplate = TemplateKind(Kind::variadic, Kind::type);

template<class T> class TemplateTemplate;

typedef TemplateTemplate<void> Template;

template<class T> class TypeTemplate;

typedef TypeTemplate<void> Type;

class Tyco
{
public:
    Tyco() { }
    String toString() const { return _implementation->toString(); }
    bool valid() const { return _implementation.valid(); }
    bool operator==(const Tyco& other) const
    {
        if (_implementation == other._implementation)
            return true;
        return _implementation->equals(other._implementation);
    }
    bool operator!=(const Tyco& other) const { return !operator==(other); }
    int hash() const { return _implementation->hash(); }
    Kind kind() const { return _implementation->kind(); }
    bool isInstantiation() const { return _implementation->isInstantiation(); }
    Tyco generatingTemplate() const
    {
        return _implementation->generatingTemplate();
    }
    Tyco templateArgument() const
    {
        return _implementation->templateArgument();
    }
protected:
    class Implementation : public ReferenceCounted
    {
    public:
        virtual String toString() const = 0;
        virtual bool equals(const Implementation* other) const
        {
            return false;
        }
        virtual Kind kind() const = 0;
        virtual int hash() const { return reinterpret_cast<int>(this); }
        virtual bool isInstantiation() const { return false; }
        virtual Tyco generatingTemplate() const { throw Exception(); }
        virtual Tyco templateArgument() const { throw Exception(); }
    };
    Tyco(const Implementation* implementation)
      : _implementation(implementation) { }
    ConstReference<Implementation> _implementation;

    friend class TemplateTemplate<void>;
    friend class EnumerationType;
    friend class StructuredType;
};

template<class T> class TypeTemplate : public Tyco
{
public:
    TypeTemplate() { }
    TypeTemplate(const Tyco& tyco) : Tyco(tyco) { }

    static Type integer;
    static Type string;
    static Type boolean;
    static Type object;
    static Type label;
    static Type voidType;

    static Type array(const Type& type)
    {
        List<Tyco> arguments;
        arguments.add(type);
        return Template::array.instantiate(arguments);
    }
    static Type tuple(const List<Type>& arguments)
    {
        List<Tyco> a;
        for (auto i = arguments.begin(); i != arguments.end(); ++i)
            a.add(*i);
        return Template::tuple.instantiate(a);
    }
protected:
    class Implementation : public Tyco::Implementation
    {
    public:
        Kind kind() const { return Kind::type; }
    };
    TypeTemplate(const Implementation* implementation)
      : Tyco(implementation) { }
private:
    friend class TemplateTemplate<void>;
};

class AtomicType : public Type
{
public:
    AtomicType(String name) : Type(new Implementation(name)) { }
private:
    class Implementation : public Type::Implementation
    {
    public:
        Implementation(String name) : _name(name) { }
        String toString() const { return _name; }
    private:
        String _name;
    };
};

Type Type::integer = AtomicType("Integer");
Type Type::string = AtomicType("String");
Type Type::boolean = AtomicType("Boolean");
Type Type::object = AtomicType("Object");
Type Type::label = AtomicType("Label");
Type Type::voidType = AtomicType("Void");

template<class T> class TemplateTemplate : public Tyco
{
public:
    TemplateTemplate(const String& name, const Kind& kind)
      : Tyco(new UninstantiatedImplementation(name, kind)) { }
    Tyco instantiate(const List<Tyco>& arguments) const
    {
        Tyco t = *this;
        for (auto i = arguments.begin(); i != arguments.end(); ++i) {
            ConstReference<Implementation> ti =
                t._implementation.referent<Implementation>();
            if (!ti.valid())
                throw Exception(String("Can't instantiate ") + t.toString());
            t = ti->instantiate(*i);
        }
        return t;
    }

    static Template array;
    static Template tuple;
private:
    class Implementation : public Tyco::Implementation
    {
    public:
        virtual Tyco instantiate(const Tyco& tyco) const = 0;
        Tyco instantiate(const TemplateKind& kind, const Tyco& tyco) const
        {
            if (_instantiations.hasKey(tyco))
                return _instantiations[tyco];
            if (kind.first() != Kind::variadic) {
                if (kind.first() != tyco.kind())
                    throw Exception(String("Can't instantiate ") + toString() +
                        String(" (argument kind ") + kind.first().toString() +
                        String(") with ") + tyco.toString() +
                        String(" (kind ") + tyco.kind().toString());
                Kind rest = kind.rest();
                Tyco instantiation;
                if (rest == Kind::type)
                    instantiation =
                        Type(new InstantiatedImplementation(this, tyco));
                else
                    instantiation = Tyco(
                        new PartiallyInstantiatedImplementation(this, tyco));
                _instantiations.add(tyco, instantiation);
                return instantiation;
            }
            Tyco instantiation(new VariadicImplementation(this, tyco));
            _instantiations.add(tyco, instantiation);
            return instantiation;
        }
        virtual String toString2(bool* needComma) const = 0;
    private:
        mutable HashTable<Tyco, Tyco> _instantiations;
    };
    class UninstantiatedImplementation : public Implementation
    {
    public:
        UninstantiatedImplementation(const String& name, const Kind& kind)
          : _name(name), _kind(kind) { }
        Kind kind() const { return _kind; }
        String toString() const { return _name; }
        Tyco instantiate(const Tyco& tyco) const
        {
            return Implementation::instantiate(_kind, tyco);
        }
    protected:
        String toString2(bool* needComma) const
        {
            *needComma = false;
            return _name + "<";
        }
    private:
        String _name;
        Kind _kind;
    };
    class PartiallyInstantiatedImplementation : public Implementation
    {
    public:
        PartiallyInstantiatedImplementation(const Implementation* parent,
            Tyco argument)
          : _parent(parent), _argument(argument) { }
        Kind kind() const { return TemplateKind(_parent->kind()).rest(); }
        String toString() const
        {
            bool needComma;
            return toString2(&needComma) + ">";
        }
        String toString2(bool* needComma) const
        {
            String s = _parent->toString2(needComma);
            if (*needComma)
                s += ", ";
            s += _argument.toString();
            *needComma = true;
            return s;
        }
        Tyco instantiate(const Tyco& tyco) const
        {
            return Implementation::instantiate(kind(), tyco);
        }
        virtual bool isInstantiation() const { return true; }
        virtual Tyco generatingTemplate() const { return Tyco(_parent); }
        virtual Tyco templateArgument() const { return _argument; }
    private:
        const Implementation* _parent;
        Tyco _argument;
        mutable HashTable<Tyco, Tyco> _instantiations;
    };
    class VariadicImplementation : public PartiallyInstantiatedImplementation
    {
    public:
        VariadicImplementation(const Implementation* parent,
            const Tyco& argument)
          : PartiallyInstantiatedImplementation(parent, argument) { }
        Kind kind() const { return Kind::variadicTemplate; }
    };
    class InstantiatedImplementation : public Type::Implementation
    {
    public:
        InstantiatedImplementation(const Template::Implementation* parent,
            Tyco argument)
          : _parent(parent), _argument(argument) { }
        String toString() const
        {
            bool needComma;
            String s = _parent->toString2(&needComma);
            if (needComma)
                s += ", ";
            return s + _argument.toString() + ">";
        }

        virtual bool isInstantiation() const { return true; }
        virtual Tyco generatingTemplate() const { return Tyco(_parent); }
        virtual Tyco templateArgument() const { return _argument; }
    private:
        const Template::Implementation* _parent;
        Tyco _argument;
    };
};

Template Template::array("Array", TemplateKind(Kind::type, Kind::type));
Template Template::tuple("Tuple", Kind::variadicTemplate);

class PointerType : public Type
{
public:
    PointerType(const Type& referent) : Type(new Implementation(referent)) { }
private:
    class Implementation : public Type::Implementation
    {
    public:
        Implementation(const Type &referent) : _referent(referent) { }
        String toString() const { return _referent.toString() + "*"; }
        bool equals(const Type::Implementation* other) const
        {
            const Implementation* o =
                dynamic_cast<const Implementation*>(other);
            if (o == 0)
                return false;
            return _referent == o->_referent;
        }
        int hash() const { return _referent.hash()*67 + 1; }
    private:
        Type _referent;
    };
};

class FunctionType : public Type
{
public:
    FunctionType(const Type& returnType, const List<Type>& parameterTypes)
      : Type(new Implementation(returnType, parameterTypes)) { }
private:
    class Implementation : public Type::Implementation
    {
    public:
        Implementation(const Type& returnType,
            const List<Type>& parameterTypes)
          : _returnType(returnType), _parameterTypes(parameterTypes) { }
        String toString() const
        {
            String s = _returnType.toString() + "(";
            for (int i = 0; i < _parameterTypes.count(); ++i) {
                if (i > 0)
                    s += ", ";
                s += _parameterTypes[i].toString();
            }
            return s + ")";
        }
        bool equals(const Type::Implementation* other) const
        {
            const Implementation* o =
                dynamic_cast<const Implementation*>(other);
            if (o == 0)
                return false;
            if (_returnType != o->_returnType)
                return false;
            int c = _parameterTypes.count();
            if (c != o->_parameterTypes.count())
                return false;
            for (int i = 0; i < c; ++i)
                if (_parameterTypes[i] != o->_parameterTypes[i])
                    return false;
            return true;
        }
        int hash() const
        {
            int h = _returnType.hash()*67 + 2;
            for (int i = 0; i < _parameterTypes.count(); ++i)
                h = h*67 + _parameterTypes[i].hash();
            return h;
        }
    private:
        Type _returnType;
        Array<Type> _parameterTypes;
    };
};

class EnumerationType : public Type
{
public:
    class Value
    {
    public:
        template<class T> Value(String name, const T& value)
            : _name(name), _value(value)
        { }
        String name() const { return _name; }
        template<class T> T value() const { return _value.value<T>(); }
        Any value() const { return _value; }
    private:
        String _name;
        Any _value;
    };

    EnumerationType(const Type& other)
      : Type(other._implementation.referent<Implementation>()) { }
    EnumerationType(String name, List<Value> values)
      : Type(new Implementation(name, values)) { }
    const Array<Value>* values() const
    {
        return _implementation.referent<Implementation>()->values();
    }
private:
    class Implementation : public Type::Implementation
    {
    public:
        Implementation(String name, List<Value> values)
          : _name(name), _values(values) { }
        String toString() const { return _name; }
        const Array<Value>* values() const { return &_values; }
    private:
        String _name;
        Array<Value> _values;
    };
};

class StructuredType : public Type
{
public:
    class Member
    {
    public:
        Member(String name, Type type) : _name(name), _type(type) { }
        String name() const { return _name; }
        Type type() const { return _type; }
        bool operator==(const Member& other) const
        {
            return _name == other._name && _type == other._type;
        }
        bool operator!=(const Member& other) const
        {
            return !operator==(other);
        }
    private:
        String _name;
        Type _type;
    };

    StructuredType(const Type& other)
      : Type(other._implementation.referent<Implementation>()) { }
    StructuredType(String name, List<Member> members)
      : Type(new Implementation(name, members)) { }
    const HashTable<String, int>* names() const
    {
        return implementation()->names();
    }
    const Array<Member>* members() const
    {
        return implementation()->members();
    }
private:
    class Implementation : public Type::Implementation
    {
    public:
        Implementation(String name, List<Member> members)
          : _name(name), _members(members)
        {
            int n = 0;
            for (auto i = members.begin(); i != members.end(); ++i) {
                _names.add(i->name(), n);
                ++n;
            }
        }
        String toString() const { return _name; }
        const HashTable<String, int>* names() const { return &_names; }
        const Array<Member>* members() const { return &_members; }
    private:
        String _name;
        HashTable<String, int> _names;
        Array<Member> _members;
    };
    const Implementation* implementation() const
    {
        return _implementation.referent<Implementation>();
    }
};

class TypedValue
{
public:
    TypedValue() { }
    TypedValue(Type type, Any defaultValue = Any(), Span span = Span())
        : _type(type), _value(defaultValue), _span(span) { }
    Type type() const { return _type; }
    Any value() const { return _value; }
    template<class T> T value() const { return _value.value<T>(); }
    void setValue(Any value) { _value = value; }
    Span span() const { return _span; }
    bool valid() const { return _value.valid(); }
private:
    Type _type;
    Any _value;
    Span _span;
};

class Conversion
{
public:
    class Implementation : public ReferenceCounted
    {
    public:
        virtual TypedValue convert(const TypedValue& value) const = 0;
        virtual bool valid() const { return true; }
    };
    Conversion() { }
    explicit Conversion(Implementation* implementation)
      : _implementation(implementation) { }
    bool valid() const
    {
        return _implementation.valid() && _implementation->valid();
    }
    TypedValue operator()(const TypedValue& value) const
    {
        return _implementation->convert(value);
    }
    ConstReference<Implementation> _implementation;
};

class TypeConverter;

class ConversionSource
{
public:
    class Implementation : public ReferenceCounted
    {
    public:
        virtual Conversion conversion(const Type& from, const Type& to,
            TypeConverter* typeConverter) const = 0;
    };
    ConversionSource() { }
    ConversionSource(Implementation* implementation)
      : _implementation(implementation) { }
    bool valid() const { return _implementation.valid(); }
    Conversion conversion(const Type& from, const Type& to,
        TypeConverter* typeConverter) const
    {
        return _implementation->conversion(from, to, typeConverter);
    }
private:
    ConstReference<Implementation> _implementation;
};

class TypeConverter
{
public:
    void addConversionSource(const Template& templateTyco,
        const ConversionSource& conversionSource)
    {
        _conversionSources.add(templateTyco, conversionSource);
    }
    void addConversion(const Type& from, const Type& to,
        const Conversion& conversion)
    {
        _conversions.add(TypePair(from, to), conversion);
    }
    Conversion conversion(const Type& from, const Type& to)
    {
        if (from == to)
            return _trivialConversion;
        TypePair pair(from, to);
        if (_conversions.hasKey(pair))
            return _conversions[pair];
        Tyco tyco = from;
        bool doneBoth = false;
        do {
            while (tyco.isInstantiation()) {
                tyco = tyco.generatingTemplate();
                if (_conversionSources.hasKey(tyco)) {
                    Conversion conversion =
                        _conversionSources[tyco].conversion(from, to, this);
                    if (conversion.valid()) {
                        _conversions.add(pair, conversion);
                        return conversion;
                    }
                }
            }
            if (doneBoth)
                break;
            tyco = to;
            doneBoth = true;
        } while (true);
        StructuredType fromStructure(from);
        StructuredType toStructure(to);
        if (fromStructure.valid() && toStructure.valid() &&
            fromStructure.toString().empty()) {
            Reference<StructuredConversionImplementation>
                implementation(new StructuredConversionImplementation(to));
            const Array<StructuredType::Member>* fromMembers =
                fromStructure.members();
            const HashTable<String, int>* fromNames = fromStructure.names();
            const Array<StructuredType::Member>* toMembers =
                toStructure.members();
            const HashTable<String, int>* toNames = toStructure.names();

            int n = toNames->count();
            for (int i = 0; i < n; ++i) {
                StructuredType::Member member = (*toMembers)[i];
                String name = member.name();
                Type toType = member.type();
                String number(decimal(i));
                int fromIndex;
                if (fromNames->hasKey(name)) {
                    fromIndex = (*fromNames)[name];
                    if (fromNames->hasKey(number))
                        return Conversion(
                            new MultipleDefinitionFailure(from, to, i, name));
                }
                else
                    if (fromNames->hasKey(number))
                        fromIndex = (*fromNames)[number];
                    else {
                        return Conversion(
                            new MissingDefinitionFailure(from, to, name));
                    }
                Type fromType = (*fromMembers)[fromIndex].type();
                Conversion c = conversion(fromType, toType);
                if (!c.valid())
                    return Conversion(
                        new MemberConversionFailure(from, to, name, c));
                implementation->add(name, c);
            }
            Conversion conversion(implementation);
            _conversions.add(TypePair(from, to), conversion);
            return conversion;
        }
        return Conversion(new ConversionFailureImplementation(from, to));
    }
    class ConversionFailureImplementation : public Conversion::Implementation
    {
    public:
        ConversionFailureImplementation(const Type& from, const Type& to)
          : _from(from), _to(to) { }
        TypedValue convert(const TypedValue& value) const
        {
            value.span().throwError(toString(value));
            return TypedValue();
        }
        virtual String sub(const TypedValue& value) const { return ""; }
        String toString(const TypedValue& value) const
        {
            String r("No conversion");
            String f = _from.toString();
            if (f != "")
                r += " from type " + f;
            r += " to type " + _to.toString() + " is available";
            String s = sub(value);
            if (s == "")
                r += ".";
            else
                r += ": " + s;
            return r;
        }
        bool valid() const { return false; }
    private:
        Type _from;
        Type _to;
    };
private:
    class TrivialConversionImplementation : public Conversion::Implementation
    {
    public:
        TypedValue convert(const TypedValue& value) const { return value; }
    };
    class StructuredConversionImplementation
      : public Conversion::Implementation
    {
    public:
        StructuredConversionImplementation(const Type& type) : _type(type) { }
        void add(const String& name, const Conversion& conversion)
        {
            _conversions[name] = conversion;
        }
        TypedValue convert(const TypedValue& value) const
        {
            auto input = value.value<Value<HashTable<String, TypedValue>>>();
            Value<HashTable<String, TypedValue>> output;
            for (HashTable<String, TypedValue>::Iterator i = input->begin();
                i != input->end(); ++i) {
                String name = i.key();
                (*output)[name] = _conversions[name]((*input)[name]);
            }
            return TypedValue(_type, output, value.span());
        }
    private:
        Type _type;
        HashTable<String, Conversion> _conversions;
    };
    class MultipleDefinitionFailure : public ConversionFailureImplementation
    {
    public:
        MultipleDefinitionFailure(const Type& from, const Type& to, int i,
            const String& name)
          : ConversionFailureImplementation(from, to), _i(i), _name(name) { }
        String sub(const TypedValue& value) const
        {
            auto input = value.value<Value<HashTable<String, TypedValue>>>();
            return "Member " + _name + " defined at " +
                (*input)[_name].span().toString() + " is already defined at " +
                (*input)[decimal(_i)].span().toString() + ".";
        }
    private:
        int _i;
        String _name;
    };
    class MissingDefinitionFailure : public ConversionFailureImplementation
    {
    public:
        MissingDefinitionFailure(const Type& from, const Type& to,
            const String& name)
          : ConversionFailureImplementation(from, to), _name(name) { }
        String sub(const TypedValue& value) const
        {
            return "Member " + _name + " is not defined.";
        }
    private:
        String _name;
    };
    class MemberConversionFailure : public ConversionFailureImplementation
    {
    public:
        MemberConversionFailure(const Type& from, const Type& to,
            const String& name, const Conversion& conversion)
          : ConversionFailureImplementation(from, to), _name(name),
            _conversion(conversion) { }
        String sub(const TypedValue& value) const
        {
            auto input = value.value<Value<HashTable<String, TypedValue>>>();
            ConstReference<ConversionFailureImplementation> i = _conversion.
                _implementation.referent<ConversionFailureImplementation>();
            String r = "For child member " + _name;
            if (i != 0)
                r += ": " + i->toString((*input)[_name]);
            return r + ".";
        }
    private:
        String _name;
        Conversion _conversion;
    };

    class TypePair
    {
    public:
        TypePair() { }
        TypePair(const Type& from, const Type& to) : _from(from), _to(to) { }
        bool operator==(const TypePair& other) const
        {
            return _from == other._from && _to == other._to;
        }
        int hash() const { return _from.hash() * 67 + _to.hash(); }
    private:
        Type _from;
        Type _to;
    };
    HashTable<TypePair, Conversion> _conversions;
    HashTable<Tyco, ConversionSource> _conversionSources;
    static Conversion _trivialConversion;
};

Conversion TypeConverter::_trivialConversion(
    new TypeConverter::TrivialConversionImplementation);

class ArrayConversionSourceImplementation
  : public ConversionSource::Implementation
{
public:
    virtual Conversion conversion(const Type& from, const Type& to,
        TypeConverter* typeConverter) const
    {
        Tyco toGenerator = to;
        if (toGenerator.isInstantiation())
            toGenerator = toGenerator.generatingTemplate();
        if (toGenerator != Template::array)
            return Conversion(new NotArrayConversionFailure(from, to));
        Type contained = to.templateArgument();

        Tyco fromGenerator = from;
        List<Conversion> conversions;
        int i = 0;
        while (fromGenerator.isInstantiation()) {
            Type argument = fromGenerator.templateArgument();
            Conversion conversion =
                typeConverter->conversion(argument, contained);
            if (!conversion.valid())
                return Conversion(
                    new ElementConversionFailure(from, to, i, conversion));
            conversions.add(conversion);
            fromGenerator = fromGenerator.generatingTemplate();
        }
        if (fromGenerator != Template::tuple)
            return Conversion(new NotTupleConversionFailure(from, to));
        return Conversion(new ArrayConversionImplementation(
            Type::array(contained), conversions));
    }
private:
    class ArrayConversionImplementation : public Conversion::Implementation
    {
    public:
        ArrayConversionImplementation(const Type& type,
            const List<Conversion>& conversions)
          : _type(type), _conversions(conversions) { }
        TypedValue convert(const TypedValue& value) const
        {
            List<TypedValue> list = value.value<List<TypedValue>>();
            List<TypedValue> results;
            int p = 0;
            for (auto i = list.begin(); i != list.end(); ++i) {
                results.add(_conversions[p](*i));
                ++p;
            }
            return TypedValue(_type, results, value.span());
        }
    private:
        Type _type;
        Array<Conversion> _conversions;
    };
    class NotArrayConversionFailure :
        public TypeConverter::ConversionFailureImplementation
    {
    public:
        NotArrayConversionFailure(const Type& from, const Type& to)
          : ConversionFailureImplementation(from, to) { }
        String sub(const TypedValue& value) const
        {
            return "Not a conversion to an Array.";
        }
    };
    class NotTupleConversionFailure :
        public TypeConverter::ConversionFailureImplementation
    {
    public:
        NotTupleConversionFailure(const Type& from, const Type& to)
          : ConversionFailureImplementation(from, to) { }
        String sub(const TypedValue& value) const
        {
            return "Not a conversion from a Tuple.";
        }
    };
    class ElementConversionFailure :
        public TypeConverter::ConversionFailureImplementation
    {
    public:
        ElementConversionFailure(const Type& from, const Type& to,
            int i, const Conversion& conversion)
          : ConversionFailureImplementation(from, to), _i(i),
            _conversion(conversion) { }
        String sub(const TypedValue& value) const
        {
            auto input = value.value<List<TypedValue>>();
            auto iterator = input.begin();
            for (int j = 0; j < _i; ++j)
                ++iterator;
            ConstReference<ConversionFailureImplementation> i = _conversion.
                _implementation.referent<ConversionFailureImplementation>();
            return String("For element ") + _i + ": " + i->toString(*iterator);
        }
    private:
        int _i;
        Conversion _conversion;
    };
};

#endif // INCLUDED_TYPE_H