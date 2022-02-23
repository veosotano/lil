#include "shared/LILShared.h"
#include "parser/LILAbstractParserReceiver.h"
#include "parser/LILCodeParser.h"
#include "shared/LILPlatformSupport.h"

using namespace LIL;

std::vector<NodeType> nodeTypes;
std::string result;

std::string NodeTypeToClassName(NodeType nodeType)
{
    switch (nodeType) {
        case NodeTypeRoot:
            return "root";
        case NodeTypeNull:
            return "null";
        case NodeTypeNegation:
            return "negation";
        case NodeTypeBoolLiteral:
            return "bool";
        case NodeTypeNumberLiteral:
            return "number_literal";
        case NodeTypeStringLiteral:
            return "string";
        case NodeTypeStringFunction:
            return "string_function";
        case NodeTypeCStringLiteral:
            return "c_string";
        case NodeTypePercentage:
            return "percentage";
        case NodeTypeExpression:
            return "expression";
        case NodeTypeUnaryExpression:
            return "unary_expression";
        case NodeTypeVarName:
            return "var_name";
        case NodeTypeType:
            return "type";
        case NodeTypeFunctionType:
            return "funtion_type";
        case NodeTypeObjectType:
            return "object_type";
        case NodeTypePointerType:
            return "pointer_type";
        case NodeTypeMultipleType:
            return "multiple_type";
        case NodeTypeStaticArrayType:
            return "static_array_type";
        case NodeTypeVarDecl:
            return "var_declaration";
        case NodeTypeAliasDecl:
            return "alias_declaration";
        case NodeTypeTypeDecl:
            return "type_declaration";
        case NodeTypeConversionDecl:
            return "conversion_declaration";
        case NodeTypeAssignment:
            return "assignment";
        case NodeTypeValuePath:
            return "value_path";
        case NodeTypePropertyName:
            return "property_name";
        case NodeTypeSelector:
            return "selector";
        case NodeTypeCombinator:
            return "combinator";
        case NodeTypeFilter:
            return "filter";
        case NodeTypeFlag:
            return "flag";
        case NodeTypeSimpleSelector:
            return "simple_selector";
        case NodeTypeSelectorChain:
            return "selector_chain";
        case NodeTypeRule:
            return "rule";
        case NodeTypeClassDecl:
            return "class_declaration";
        case NodeTypeObjectDefinition:
            return "object_definition";
        case NodeTypeComment:
            return "comment";
        case NodeTypeInstruction:
            return "instruction";
        case NodeTypeIfInstruction:
            return "if_instruction";
        case NodeTypeFunctionDecl:
            return "function_declaration";
        case NodeTypeFunctionCall:
            return "function_call";
        case NodeTypeIndexAccessor:
            return "index_accessor";
        case NodeTypeFlowControl:
            return "flow_control";
        case NodeTypeFlowControlCall:
            return "flow_control_call";
        case NodeTypeForeignLang:
            return "foreign_language";
        case NodeTypeValueList:
            return "value_list";
            
        default:
            return "ERROR: unknown node type";
    }
}

class LILColorizerReceiver : public LILAbstractParserReceiver
{
public:
    LILColorizerReceiver();
    virtual ~LILColorizerReceiver();
    void receiveNodeStart(NodeType nodeType) override;
    void receiveNodeEnd(NodeType nodeType) override;
    void receiveNodeCommit() override;
    void receiveNodeData(ParserEvent eventType, const LILString & data) override;
    void receiveError(LILString message, LILString file, size_t startLine, size_t startCol) override;
};

LILColorizerReceiver::LILColorizerReceiver()
{
    
}

LILColorizerReceiver::~LILColorizerReceiver()
{
    
}

void LILColorizerReceiver::receiveNodeStart(NodeType nodeType)
{
    nodeTypes.push_back(nodeType);
}

void LILColorizerReceiver::receiveNodeEnd(NodeType nodeType)
{
    nodeTypes.pop_back();
}

void LILColorizerReceiver::receiveNodeCommit()
{
    
}

void LILColorizerReceiver::receiveNodeData(ParserEvent eventType, const LILString & data)
{
    if (eventType == ParserEventWhitespace)
    {
        result += data.data();
    }
    else if (eventType == ParserEventComment)
    {
        result += "<span class=\"comment\">" + data.data() + "</span>";
    }
    else if (eventType == ParserEventPunctuation)
    {
        result += "<span class=\"punctuation\">" + data.data() + "</span>";
    }
    else if (data.length() > 0)
    {
        result += "<span class=\""+NodeTypeToClassName(nodeTypes.back())+"\">" + data.data() + "</span>";
    }
}

void LILColorizerReceiver::receiveError(LILString message, LILString file, size_t startLine, size_t startCol)
{
    std::cout << message.data();
}

int main(int argc, const char * argv[]) {
    if (argc == 1) {
        std::cout << "\nERROR: Needs an input filename\n\n";
        return -1;
    }

    bool verbose = false;
    std::string inName;
    for (int i = 0; i<argc; i+=1) {
        std::string command = argv[i];
        if (command == "-v" || command == "--verbose") {
            //verbose output
            verbose = true;
        } else {
            //anything else is interpreted as the input filename
            inName = command;
        }
    }
    
    nodeTypes.push_back(NodeTypeInvalid);

    result += "<html>\n";
    result += "<head>\n";
    result += "<link rel=\"stylesheet\" href=\"style.css\" />";
    result += "</head>\n";
    result += "<body><pre>\n";
    
    
    std::string directory = LIL_getCurrentDir();
    
    std::ifstream file(directory+"/"+inName, std::ios::in);
    if (file.fail()) {
        std::cout << "\nERROR: Failed to read the file "+inName;
        return -1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    
    LILString lilStr(buffer.str());
    
    LILColorizerReceiver receiver;
    LILCodeParser parser(&receiver);
    parser.parseString(lilStr);
    
    result += "</pre></body>\n";
    result += "</html>\n";
    
    std::cout << result;
    
    return 0;
}
