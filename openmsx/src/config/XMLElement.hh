// $Id: XMLElement.hh 12622 2012-06-14 20:11:33Z m9710797 $

#ifndef XMLELEMENT_HH
#define XMLELEMENT_HH

#include "serialize_constr.hh"
#include "serialize_meta.hh"
#include <utility>
#include <string>
#include <vector>
#include <memory>

namespace StringOp { class Builder; }

namespace openmsx {

class FileContext;

class XMLElement
{
public:
	//
	// Basic functions
	//

	// construction, destruction, copy, assign
	explicit XMLElement(string_ref name);
	XMLElement(string_ref name, string_ref data);
	XMLElement(const XMLElement& element);
	XMLElement& operator=(const XMLElement& element);
	~XMLElement();

	// name
	const std::string& getName() const { return name; }
	void setName(string_ref name);
	void clearName();

	// data
	const std::string& getData() const { return data; }
	void setData(string_ref data);

	// attribute
	void addAttribute(string_ref name, string_ref value);
	void setAttribute(string_ref name, string_ref value);
	void removeAttribute(string_ref name);

	// child
	typedef std::vector<XMLElement*> Children;
	void addChild(std::auto_ptr<XMLElement> child);
	std::auto_ptr<XMLElement> removeChild(const XMLElement& child);
	const Children& getChildren() const { return children; }
	bool hasChildren() const { return !children.empty(); }

	//
	// Convenience functions
	//

	// data
	bool getDataAsBool() const;
	int getDataAsInt() const;
	double getDataAsDouble() const;

	// attribute
	bool hasAttribute(string_ref name) const;
	const std::string& getAttribute(string_ref attName) const;
	string_ref  getAttribute(string_ref attName,
	                         string_ref defaultValue) const;
	bool getAttributeAsBool(string_ref attName,
	                        bool defaultValue = false) const;
	int getAttributeAsInt(string_ref attName,
	                      int defaultValue = 0) const;
	bool findAttributeInt(string_ref attName,
	                      unsigned& result) const;

	// child
	const XMLElement* findChild(string_ref name) const;
	XMLElement* findChild(string_ref name);
	const XMLElement& getChild(string_ref name) const;
	XMLElement& getChild(string_ref name);

	const XMLElement* findChildWithAttribute(
		string_ref name, string_ref attName,
		string_ref attValue) const;
	XMLElement* findChildWithAttribute(
		string_ref name, string_ref attName,
		string_ref attValue);
	const XMLElement* findNextChild(string_ref name,
	                                unsigned& fromIndex) const;

	void getChildren(string_ref name, Children& result) const;

	XMLElement& getCreateChild(string_ref name,
	                           string_ref defaultValue = "");
	XMLElement& getCreateChildWithAttribute(
		string_ref name, string_ref attName,
		string_ref attValue);

	const std::string& getChildData(string_ref name) const;
	string_ref getChildData(string_ref name,
	                        string_ref defaultValue) const;
	bool getChildDataAsBool(string_ref name,
	                        bool defaultValue = false) const;
	int getChildDataAsInt(string_ref name,
	                      int defaultValue = 0) const;
	void setChildData(string_ref name, string_ref value);

	void removeAllChildren();

	// various
	std::string dump() const;
	static std::string XMLEscape(const std::string& str);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

	// For backwards compatibility with older savestates
	static std::auto_ptr<FileContext> getLastSerializedFileContext();

private:
	typedef std::pair<std::string, std::string> Attribute;
	typedef std::vector<Attribute> Attributes;
	Attributes::iterator findAttribute(string_ref name);
	Attributes::const_iterator findAttribute(string_ref name) const;
	void dump(StringOp::Builder& result, unsigned indentNum) const;

	std::string name;
	std::string data;
	Children children;
	Attributes attributes;
};
SERIALIZE_CLASS_VERSION(XMLElement, 2);

template<> struct SerializeConstructorArgs<XMLElement>
{
	typedef Tuple<std::string, std::string> type;
	template<typename Archive> void save(Archive& ar, const XMLElement& xml)
	{
		ar.serialize("name", xml.getName());
		ar.serialize("data", xml.getData());
	}
	template<typename Archive> type load(Archive& ar, unsigned /*version*/)
	{
		std::string name, data;
		ar.serialize("name", name);
		ar.serialize("data", data);
		return make_tuple(name, data);
	}
};

} // namespace openmsx

#endif
