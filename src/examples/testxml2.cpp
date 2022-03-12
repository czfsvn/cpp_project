#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "boost_asio.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <vector>

using namespace std;

const std::string phonebook = "../configs/phonebook.xml";
const std::string jobxml    = "../configs/gjob.xml";

namespace parsexml
{
    //解析每一个phone，提取出name、tel、address
    static int parse_phone(xmlDocPtr doc, xmlNodePtr cur)
    {
        assert(doc || cur);
        xmlChar* key;

        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            //获取name
            if ((!xmlStrcmp(cur->name, ( const xmlChar* )"name")))
            {
                key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                printf("name: %s\t", key);
                xmlFree(key);
            }
            //获取tel
            if ((!xmlStrcmp(cur->name, ( const xmlChar* )"tel")))
            {
                key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                printf("tel: %s\t", key);
                xmlFree(key);
            }
            //获取address
            if ((!xmlStrcmp(cur->name, ( const xmlChar* )"address")))
            {
                key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                printf("address: %s\n", key);
                xmlFree(key);
            }
            cur = cur->next;
        }
        return 0;
    }

    static int parse_phone_book(const char* file_name)
    {
        assert(file_name);

        xmlDocPtr  doc;  // xml整个文档的树形结构
        xmlNodePtr cur;  // xml节点
        xmlChar*   id;   // phone id

        //获取树形结构
        doc = xmlParseFile(file_name);
        if (doc == NULL)
        {
            fprintf(stderr, "Failed to parse xml file:%s\n", file_name);
            goto FAILED;
        }

        //获取根节点
        cur = xmlDocGetRootElement(doc);
        if (cur == NULL)
        {
            fprintf(stderr, "Root is empty.\n");
            goto FAILED;
        }

        if ((xmlStrcmp(cur->name, ( const xmlChar* )"phone_books")))
        {
            fprintf(stderr, "The root is not phone_books.\n");
            goto FAILED;
        }

        //遍历处理根节点的每一个子节点
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, ( const xmlChar* )"phone")))
            {
                id = xmlGetProp(cur, ( xmlChar* )"id");
                printf("id:%s\t", id);
                parse_phone(doc, cur);
            }
            cur = cur->next;
        }
        xmlFreeDoc(doc);
        return 0;
    FAILED:
        if (doc)
        {
            xmlFreeDoc(doc);
        }
        return -1;
    }

    void main()
    {
        if (parse_phone_book(phonebook.c_str()) != 0)
        {
            fprintf(stderr, "Failed to parse phone book.\n");
            return;
        }
    }

}  // namespace parsexml

namespace libxml2_example
{
#define DEBUG(x) printf(x)

    /*
     * A person record
     * an xmlChar * is really an UTF8 encoded char string (0 terminated)
     */
    typedef struct person
    {
        xmlChar* name;
        xmlChar* email;
        xmlChar* company;
        xmlChar* organisation;
        xmlChar* smail;
        xmlChar* webPage;
        xmlChar* phone;
    } person, *personPtr;

    /*
     * And the code needed to parse it
     */
    static personPtr parsePerson(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur)
    {
        personPtr ret = NULL;

        DEBUG("parsePerson\n");
        /*
         * allocate the struct
         */
        ret = ( personPtr )malloc(sizeof(person));
        if (ret == NULL)
        {
            fprintf(stderr, "out of memory\n");
            return (NULL);
        }
        memset(ret, 0, sizeof(person));

        /* We don't care what the top level element name is */
        /* COMPAT xmlChildrenNode is a macro unifying libxml1 and libxml2 names */
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, ( const xmlChar* )"Person")) && (cur->ns == ns))
                ret->name = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if ((!xmlStrcmp(cur->name, ( const xmlChar* )"Email")) && (cur->ns == ns))
                ret->email = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            cur = cur->next;
        }

        return (ret);
    }

    /*
     * and to print it
     */
    static void printPerson(personPtr cur)
    {
        if (cur == NULL)
            return;
        printf("------ Person\n");
        if (cur->name)
            printf("	name: %s\n", cur->name);
        if (cur->email)
            printf("	email: %s\n", cur->email);
        if (cur->company)
            printf("	company: %s\n", cur->company);
        if (cur->organisation)
            printf("	organisation: %s\n", cur->organisation);
        if (cur->smail)
            printf("	smail: %s\n", cur->smail);
        if (cur->webPage)
            printf("	Web: %s\n", cur->webPage);
        if (cur->phone)
            printf("	phone: %s\n", cur->phone);
        printf("------\n");
    }

    /*
     * a Description for a Job
     */
    typedef struct job
    {
        xmlChar*  projectID;
        xmlChar*  application;
        xmlChar*  category;
        personPtr contact;
        int       nbDevelopers;
        personPtr developers[100]; /* using dynamic alloc is left as an exercise */
    } job, *jobPtr;

    /*
     * And the code needed to parse it
     */
    static jobPtr parseJob(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur)
    {
        jobPtr ret = NULL;

        DEBUG("parseJob\n");
        /*
         * allocate the struct
         */
        ret = ( jobPtr )malloc(sizeof(job));
        if (ret == NULL)
        {
            fprintf(stderr, "out of memory\n");
            return (NULL);
        }
        memset(ret, 0, sizeof(job));

        /* We don't care what the top level element name is */
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {

            if ((!xmlStrcmp(cur->name, ( const xmlChar* )"Project")) && (cur->ns == ns))
            {
                ret->projectID = xmlGetProp(cur, ( const xmlChar* )"ID");
                if (ret->projectID == NULL)
                {
                    fprintf(stderr, "Project has no ID\n");
                }
            }
            if ((!xmlStrcmp(cur->name, ( const xmlChar* )"Application")) && (cur->ns == ns))
                ret->application = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if ((!xmlStrcmp(cur->name, ( const xmlChar* )"Category")) && (cur->ns == ns))
                ret->category = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if ((!xmlStrcmp(cur->name, ( const xmlChar* )"Contact")) && (cur->ns == ns))
                ret->contact = parsePerson(doc, ns, cur);
            cur = cur->next;
        }

        return (ret);
    }

    /*
     * and to print it
     */
    static void printJob(jobPtr cur)
    {
        int i;

        if (cur == NULL)
            return;
        printf("=======  Job\n");
        if (cur->projectID != NULL)
            printf("projectID: %s\n", cur->projectID);
        if (cur->application != NULL)
            printf("application: %s\n", cur->application);
        if (cur->category != NULL)
            printf("category: %s\n", cur->category);
        if (cur->contact != NULL)
            printPerson(cur->contact);
        printf("%d developers\n", cur->nbDevelopers);

        for (i = 0; i < cur->nbDevelopers; i++)
            printPerson(cur->developers[i]);
        printf("======= \n");
    }

    /*
     * A pool of Gnome Jobs
     */
    typedef struct gjob
    {
        int    nbJobs;
        jobPtr jobs[500]; /* using dynamic alloc is left as an exercise */
    } gJob, *gJobPtr;

    static gJobPtr parseGjobFile(const char* filename ATTRIBUTE_UNUSED)
    {
        xmlDocPtr  doc;
        gJobPtr    ret;
        jobPtr     curjob;
        xmlNsPtr   ns;
        xmlNodePtr cur;

#ifdef LIBXML_SAX1_ENABLED
        /*
         * build an XML tree from a the file;
         */
        doc = xmlParseFile(filename);
        if (doc == NULL)
            return (NULL);
#else
        /*
         * the library has been compiled without some of the old interfaces
         */
        return (NULL);
#endif /* LIBXML_SAX1_ENABLED */

        /*
         * Check the document is of the right kind
         */

        cur = xmlDocGetRootElement(doc);
        if (cur == NULL)
        {
            fprintf(stderr, "empty document\n");
            xmlFreeDoc(doc);
            return (NULL);
        }
        ns = xmlSearchNsByHref(doc, cur, ( const xmlChar* )"http://www.gnome.org/some-location");
        if (ns == NULL)
        {
            fprintf(stderr, "document of the wrong type, GJob Namespace not found\n");
            xmlFreeDoc(doc);
            return (NULL);
        }
        if (xmlStrcmp(cur->name, ( const xmlChar* )"Helping"))
        {
            fprintf(stderr, "document of the wrong type, root node != Helping");
            xmlFreeDoc(doc);
            return (NULL);
        }

        /*
         * Allocate the structure to be returned.
         */
        ret = ( gJobPtr )malloc(sizeof(gJob));
        if (ret == NULL)
        {
            fprintf(stderr, "out of memory\n");
            xmlFreeDoc(doc);
            return (NULL);
        }
        memset(ret, 0, sizeof(gJob));

        /*
         * Now, walk the tree.
         */
        /* First level we expect just Jobs */
        cur = cur->xmlChildrenNode;
        while (cur && xmlIsBlankNode(cur))
        {
            cur = cur->next;
        }
        if (cur == 0)
        {
            xmlFreeDoc(doc);
            free(ret);
            return (NULL);
        }
        if ((xmlStrcmp(cur->name, ( const xmlChar* )"Jobs")) || (cur->ns != ns))
        {
            fprintf(stderr, "document of the wrong type, was '%s', Jobs expected", cur->name);
            fprintf(stderr, "xmlDocDump follows\n");
#ifdef LIBXML_OUTPUT_ENABLED
            xmlDocDump(stderr, doc);
            fprintf(stderr, "xmlDocDump finished\n");
#endif /* LIBXML_OUTPUT_ENABLED */
            xmlFreeDoc(doc);
            free(ret);
            return (NULL);
        }

        /* Second level is a list of Job, but be laxist */
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, ( const xmlChar* )"Job")) && (cur->ns == ns))
            {
                curjob = parseJob(doc, ns, cur);
                if (curjob != NULL)
                    ret->jobs[ret->nbJobs++] = curjob;
                if (ret->nbJobs >= 500)
                    break;
            }
            cur = cur->next;
        }

        return (ret);
    }

    static void handleGjob(gJobPtr cur)
    {
        int i;

        /*
         * Do whatever you want and free the structure.
         */
        printf("%d Jobs registered\n", cur->nbJobs);
        for (i = 0; i < cur->nbJobs; i++)
            printJob(cur->jobs[i]);
    }

    void main(int argc, char** argv)
    {
        /* COMPAT: Do not generate nodes for formatting spaces */
        LIBXML_TEST_VERSION
        xmlKeepBlanksDefault(0);

        gJobPtr cur = parseGjobFile(jobxml.c_str());
        if (cur)
        {
            handleGjob(cur);
        }
        /*
        for (i = 1; i < argc; i++)
        {
            cur = parseGjobFile(argv[i]);
            if (cur)
                handleGjob(cur);
            else
                fprintf(stderr, "Error parsing file '%s'\n", argv[i]);
        }
        */
        /* Clean up everything else before quitting. */
        xmlCleanupParser();
    }
}  // namespace libxml2_example

namespace ns_test1
{
    struct Bus
    {
        struct Addr
        {
            std::string addr = {};

            bool parse(xmlNodePtr node)
            {
                if (!node)
                    return false;

                addr = std::string(( char* )(xmlGetProp(node, ( xmlChar* )"address")));
                std::cout << "addr: " << addr << std::endl;
                return true;
            }
        };

        uint32_t    id      = 0;
        std::string name    = {};
        uint64_t    tell    = 0;
        std::string address = {};

        std::vector<Addr> addr_vec = {};

        bool parse(xmlNodePtr node)
        {
            if (!node)
                return false;

            id      = atoi(( char* )xmlGetProp(node, ( xmlChar* )"id"));
            name    = std::string(( char* )(xmlGetProp(node, ( xmlChar* )"name")));
            tell    = atol(( char* )xmlGetProp(node, ( xmlChar* )"tel"));
            address = std::string(( char* )(xmlGetProp(node, ( xmlChar* )"address")));

            std::cout << "bus   id: " << id << ", name: " << name << ", tel: " << tell << ", addr: " << address
                      << std::endl;

            xmlNodePtr child = node->xmlChildrenNode;
            while (child)
            {
                if ((!xmlStrcmp(child->name, ( const xmlChar* )"addr")))
                {
                    Addr addr;
                    if (addr.parse(child))
                    {
                        addr_vec.emplace_back(addr);
                    }
                }
                child = child->next;
            }

            return true;
        }
    };
    void test1(const std::string& file_name)
    {
        xmlDocPtr doc = xmlParseFile(file_name.c_str());
        if (!doc)
        {
            std::cout << "open file failed\n";
            return;
        }

        xmlNodePtr cur = xmlDocGetRootElement(doc);
        if (cur == NULL)
        {
            std::cout << "get root failed\n";
            return;
        }

        if ((xmlStrcmp(cur->name, ( const xmlChar* )"phone_books")))
        {
            std::cout << "get root node failed\n";
            return;
        }
        cur = cur->xmlChildrenNode;
        while (cur)
        {
            if ((!xmlStrcmp(cur->name, ( const xmlChar* )"phone")))
            {
                const uint32_t id = atoi(( char* )xmlGetProp(cur, ( xmlChar* )"id"));
                std::string    name(( char* )(xmlGetProp(cur, ( xmlChar* )"name")));
                const uint64_t tel = atol(( char* )xmlGetProp(cur, ( xmlChar* )"tel"));
                std::string    addr(( char* )(xmlGetProp(cur, ( xmlChar* )"address")));

                std::cout << "phone id: " << id << ", name: " << name << ", tel: " << tel << ", addr: " << addr
                          << std::endl;
            }

            if ((!xmlStrcmp(cur->name, ( const xmlChar* )"bus")))
            {
                /*
                const uint32_t id = atoi((char*)xmlGetProp(cur, (xmlChar*)"id"));
                std::string    name((char*)(xmlGetProp(cur, (xmlChar*)"name")));
                const uint64_t tel = atol((char*)xmlGetProp(cur, (xmlChar*)"tel"));
                std::string    addr((char*)(xmlGetProp(cur, (xmlChar*)"address")));

                std::cout << "bus   id: " << id << ", name: " << name << ", tel: " << tel
                          << ", addr: " << addr << std::endl;
                */
                Bus bus;
                bus.parse(cur);
            }
            cur = cur->next;
        }
    }
    void main()
    {
        std::string filename = "../configs/test.xml";
        test1(filename);
        return;
    }
}  // namespace ns_test1

namespace ns_xml2
{
    void main(int argc, char** argv)
    {
        // parsexml::main();
        // libxml2_example::main(argc, argv);
        ns_test1::main();
        std::cout << "test xml2 \n";
        return;
    }
}  // namespace ns_xml2

