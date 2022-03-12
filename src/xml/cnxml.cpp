#include "cnxml.h"
#include <iconv.h>
#include <string.h>

namespace cnxml
{
    /// XmlDoc
    XmlDoc::~XmlDoc()
    {
        if (!doc_)
            return;

        xmlFreeDoc(doc_);
        doc_ = nullptr;
    }

    const char* XmlDoc::encoding() const
    {
        return ( const char* )(doc_->encoding);
    }

    bool XmlDoc::createDoc()
    {
        doc_ = xmlNewDoc(( const xmlChar* )"1.0");
        return doc_ != nullptr;
    }

    bool XmlDoc::parseFromFile(const std::string& filename)
    {
        doc_ = xmlParseFile(filename.c_str());
        return doc_ != nullptr;
    }

    bool XmlDoc::saveToFile(const std::string& filename, const std::string& encoding /* = "utf-8"*/, bool indent /*= 1*/)
    {
        return doc_ && xmlSaveFormatFileEnc(filename.c_str(), doc_, encoding.c_str(), indent) > 0;
    }

    bool XmlDoc::dump(std::string& txt, const std::string& encoding /* = "utf-8"*/, bool indent /*= 1*/)
    {
        if (!doc_)
            return false;

        xmlChar* out  = nullptr;
        int32_t  size = 0;
        xmlDocDumpFormatMemoryEnc(doc_, &out, &size, encoding.c_str(), indent);
        if (out)
        {
            txt = ( char* )out;
            xmlFree(out);
            return true;
        }

        return false;
    }

    unsigned char* XmlDoc::charConv(unsigned char* in, const char* fromEncoding, const char* toEncoding)
    {
        size_t         size    = strlen(( char* )in);
        const size_t   outsize = size * 2 + 1;
        unsigned char* out     = new unsigned char[outsize];
        memset(out, 0, outsize);
        if (out)
        {
            if (fromEncoding && toEncoding)
            {
                iconv_t icv_in = iconv_open(toEncoding, fromEncoding);
                if (icv_in == ( iconv_t )-1)
                {
                    delete[] out;
                    out = nullptr;
                }
                else
                {
                    char*  fromtmp = ( char* )in;
                    char*  totmp   = ( char* )out;
                    size_t tmpout  = outsize - 1;
                    size_t ret     = iconv(icv_in, &fromtmp, &size, &totmp, &tmpout);
                    if (ret == ( size_t )-1)
                    {
                        delete[] out;
                        out = nullptr;
                    }
                }
            }
            else
            {
                strncpy(( char* )out, ( char* )in, size);
            }
        }
        return (out);
    }

    /// XmlParser =================
    bool XmlParser::parseFile(const std::string& filename)
    {
        return xml_doc_.parseFromFile(filename);
    }

    bool XmlParser::saveToFile(const std::string& filename, const std::string& encoding /* = "utf-8"*/, bool indent /* = 1*/)
    {
        return xml_doc_.saveToFile(filename, encoding, indent);
    }
    bool XmlParser::dump(std::string& txt, const std::string& encoding /* = "utf-8"*/, bool indent /* = 1*/)
    {
        return xml_doc_.dump(txt, encoding, indent);
    }

    NodePtr XmlParser::getRootNode(const char* name /* = nullptr*/)
    {
        NodePtr curnode = xmlDocGetRootElement(xml_doc_);
        if (!curnode)
            return nullptr;

        if (name && xmlStrcmp(curnode->name, ( const xmlChar* )name))
            return nullptr;

        return curnode;
    }

    NodePtr XmlParser::checkNexNode(NodePtr node, const char* name /* = nullptr*/)
    {
        if (name)
        {
            while (node)
            {
                if (!xmlStrcmp(node->name, ( const xmlChar* )name))
                    break;

                node = node->next;
            }
        }
        else
        {
            while (node)
            {
                if (!xmlNodeIsText(node))
                    break;

                node = node->next;
            }
        }
        return node;
    }

    NodePtr XmlParser::getNextNode(NodePtr node, const char* name /* = nullptr*/)
    {
        return node ? checkNexNode(node->next, name) : nullptr;
    }

    NodePtr XmlParser::getChildNode(NodePtr node, const char* name /* = nullptr*/)
    {
        return node ? checkNexNode(node->xmlChildrenNode, name) : nullptr;
    }

    NodePtrs XmlParser::getChildNodes(NodePtr node, const char* name /* = nullptr*/)
    {
        NodePtrs childrens = {};
        if (node && node->xmlChildrenNode)
        {
            NodePtr node = node->xmlChildrenNode;
            while (node)
            {
                if (name)
                {
                    if (!xmlStrcmp(node->name, ( const xmlChar* )name))
                        childrens.emplace_back(node);
                }
                else
                {
                    if (!xmlNodeIsText(node))
                        childrens.emplace_back(node);
                }
                node = node->next;
            }
        }

        return childrens;
    }

    /// XmlWriter =================
    XmlWriter::XmlWriter()
    {
        xml_doc_.createDoc();
    }

    bool XmlWriter::save(const std::string& filename, const std::string& encoding /* = "utf-8"*/, bool indent /* = 1*/)
    {
        return xml_doc_.saveToFile(filename, encoding, indent);
    }

    bool XmlWriter::dump(std::string& text, const std::string& encoding /* = "utf-8"*/, bool indent /* = 1*/)
    {
        return xml_doc_.dump(text, encoding, indent);
    }

    NodePtr XmlWriter::createRootNode(const char* name)
    {
        if (!xml_doc_)
            return nullptr;

        NodePtr root = xmlNewNode(nullptr, ( const xmlChar* )name);
        if (!root)
            return nullptr;

        xmlDocSetRootElement(xml_doc_, root);
        return root;
    }

    NodePtr XmlWriter::createChildNode(const NodePtr parent, const char* name, const char* content)
    {
        if (!xml_doc_ || !parent)
            return nullptr;

        unsigned char* out = XmlDoc::charConv(( unsigned char* )content, "GB2312", "utf-8");
        if (!out)
            return nullptr;

        NodePtr node = xmlNewChild(parent, nullptr, ( const xmlChar* )name, ( const xmlChar* )out);
        delete[] out;
        return node;
    }

    NodePtr XmlWriter::createComment(const NodePtr parent, const char* comment)
    {
        if (!parent)
            return nullptr;

        unsigned char* out = XmlDoc::charConv(( unsigned char* )comment, "GB2312", "utf-8");
        if (!out)
            return nullptr;

        NodePtr commentNode = xmlNewComment(( const xmlChar* )out);
        NodePtr result      = commentNode ? xmlAddChild(parent, commentNode) : nullptr;
        delete[] out;
        return result;
    }

    bool XmlWriter::createNodePropStr(const NodePtr node, const char* prop, const char* value)
    {
        if (!node)
            return false;

        unsigned char* out = XmlDoc::charConv(( unsigned char* )value, "GB2312", "utf-8");
        if (!out)
            return false;

        bool ret = xmlNewProp(node, ( const xmlChar* )prop, ( const xmlChar* )out);
        delete[] out;
        return ret;
    }
}  // namespace cnxml
