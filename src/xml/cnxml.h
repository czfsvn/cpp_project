#pragma once

#include <libxml/tree.h>
#include <sstream>
#include <vector>

namespace cnxml
{
    using NodePtr  = xmlNode*;
    using NodePtrs = std::vector<NodePtr>;

    class XmlDoc
    {
    public:
        static unsigned char* charConv(unsigned char* in, const char* fromEncoding, const char* toEncoding);

        XmlDoc(xmlDoc* doc = nullptr) : doc_(doc) {}
        ~XmlDoc();

        bool parseFromFile(const std::string& filename);
        bool saveToFile(const std::string& filename, const std::string& encoding = "utf-8", bool indent = 1);
        bool dump(std::string& txt, const std::string& encoding = "utf-8", bool indent = 1);

        bool createDoc();

        operator xmlDoc*() const
        {
            return doc_;
        }

        const char* encoding() const;

    private:
        xmlDoc* doc_ = nullptr;
    };

    class XmlParser
    {
    public:
        XmlParser()  = default;
        ~XmlParser() = default;

        bool parseFile(const std::string& filename);

        NodePtr  getRootNode(const char* name = nullptr);
        NodePtr  getNextNode(NodePtr node, const char* name = nullptr);
        NodePtr  getChildNode(NodePtr node, const char* name = nullptr);
        NodePtrs getChildNodes(NodePtr node, const char* name = nullptr);

        const std::string getNodePropStr(NodePtr node, const char* prop);
        const std::string getNodeContentStr(NodePtr node);

        bool saveToFile(const std::string& filename, const std::string& encoding = "utf-8", bool indent = 1);
        bool dump(std::string& txt, const std::string& encoding = "utf-8", bool indent = 1);

        template <typename T>
        const T getNodeProp(NodePtr node, const char* prop)
        {
            return T();  // todo:  xml::cast<T, const std::string&>(getNodePropStr(node, prop));
        }

        template <typename T>
        const T getNodeContent(NodePtr node, const char* prop)
        {
            return T();  // todo: xml::cast<T, const std::string&>(getNodeContentStr(node));
        }

    private:
        NodePtr checkNexNode(NodePtr node, const char* name);

    private:
        XmlDoc xml_doc_;
    };

    class XmlWriter
    {
    public:
        XmlWriter();

        NodePtr createRootNode(const char* name);
        NodePtr createChildNode(const NodePtr parent, const char* name, const char* content);
        NodePtr createComment(const NodePtr parent, const char* comment);
        bool    createNodePropStr(const NodePtr node, const char* comment, const char* value);

        template <typename T>
        bool createNodeProp(const NodePtr node, const char* prop, const T& value)
        {
            // todo finish it
            return createNodePropStr(node, prop, "" /*xml::cast<std::string, const T&>(value).c_str()*/);
        }

        template <typename T>
        std::string ConentToText(const T& value)
        {
            return "";  // todo:    xml::cast<str::string, constT&>(value);
        }

        bool save(const std::string& filename, const std::string& encoding = "utf-8", bool indent = 1);

        bool dump(std::string& filename, const std::string& encoding = "utf-8", bool indent = 1);

    private:
        XmlDoc xml_doc_;
    };

    // xmlconfigbase

    class xml_config_base
    {
    public:
        virtual ~xml_config_base() {}

        virtual void dump(std::ostream& oss, int32_t deep = 0) const {}

        virtual bool dynamic_load() const
        {
            return true;
        }

        virtual bool dynamic_load(const std::string& filenbame) const
        {
            return true;
        }

        virtual bool dynamic_copy(xml_config_base* cfg)
        {
            return true;
        }

        virtual xml_config_base* clone()
        {
            return nullptr;
        }

        virtual const std::string getFileName() const
        {
            return "";
        }

        bool registerd = false;
    };

    template <typename Config>
    class XmlConfig : public Config, xml_config_base
    {
    private:
        std::string file_name_ = {};

    public:
        using ThisType = XmlConfig<Config>;

        XmlConfig(const std::string& filename = "") : file_name_(filename) {}

        bool load(const std::string& filename)
        {
            XmlParser xml;
            this->file_name_ = filename;
            if (!xml.parseFile(filename))
                return false;

            NodePtr root = xml.getRootNode();
            if (!root)
                return false;

            this->reset();
            return this->parse(xml, root);
        }

        bool save(const std::string& filename)
        {
            XmlWriter xml;
            NodePtr   root = xml.createRootNode("Config");
            if (root && this->write(xml, root))
                xml.save(filename, "GB2312");

            return true;
        }

        std::string dump()
        {
            std::string result;
            XmlWriter   xml;
            NodePtr     root = xml.createRootNode("Config");
            if (root && this->write(xml, root))
                xml.dump(result, "GB2312");

            return result;
        }

        std::string dumpAsText()
        {
            std::ostringstream oss;
            this->write2(oss, 0);
            return oss.str();
        }

    public:
        void dump(std::ostream& oss, int32_t deep = 0) const override
        {
            oss << const_cast<ThisType*>(this)->dumpAsText();
        }

        bool dynamic_load() override
        {
            return this->load(file_name_);
        }

        bool dynamic_load(const std::string& filename) override
        {
            return this->load(filename);
        }

        xml_config_base* clone() override
        {
            return new ThisType(getFileName());
        }

        bool dynamic_copy(xml_config_base* cfg) override
        {
            ThisType* tmp = dynamic_cast<ThisType>(cfg);
            if (!tmp)
                return false;

            std::swap(*this, *tmp);
            return true;
        }

        const std::string getFileName() const override
        {
            return file_name_;
        }
    };

    // namespace cnxml
}  // namespace cnxml