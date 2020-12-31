/********************************************************************
 *
 *      LIL Is a Language
 *
 *      AUTHORS: Miro Keller
 *
 *      COPYRIGHT: ©2020-today:  All Rights Reserved
 *
 *      LICENSE: see LICENSE file
 *
 *      This file contains the abstract base class for all nodes
 *
 ********************************************************************/

#ifndef LILNODE_H
#define LILNODE_H

#include "LILTypeEnums.h"
#include "LILBasicValues.h"
#include "LILString.h"
#include "LILClonable.h"

namespace LIL {

    class LILDocument;
    class LILType;
    class LILVarNode;
    class LILVisitor;
    
    class LILNode : public LILClonable, public std::enable_shared_from_this<LILNode>
    {
    public:
        struct SourceLocation {
            LILString file;
            size_t line;
            size_t column;
            LILRange range;
        };
        
        friend class LILASTValidator;
        friend class LILTypeGuesser;
        friend class LILTypeValidator;
        friend class LILConversionInserter;
        
        static LILString nodeTypeToString(NodeType nodeType);
        LILNode(NodeType type);
        LILNode(const LILNode &orig);
        virtual ~LILNode();
        
        virtual void accept(LILVisitor * visitor);
        
        static bool isContainerNode(NodeType nodeType);
        virtual void receiveNodeData(const LILString & data);
        virtual bool isVarNode() const;
        std::shared_ptr<LILVarNode> getClosestVarNode() const;
        virtual bool isRootNode() const;
        virtual bool isTypedNode() const;
        virtual std::shared_ptr<LILType> getType() const;
        void setSourceLocation(LILNode::SourceLocation loc);
        LILNode::SourceLocation getSourceLocation() const;
        std::shared_ptr<LILNode> clone() const;
        virtual LILString stringRep();
        std::string stdStringRep();
        virtual bool equalTo(std::shared_ptr<LILNode> otherNode);

        virtual bool isA(NodeType otherType) const;
        NodeType getNodeType() const;
        std::shared_ptr<LILNode> getParentNode() const;
        void setParentNode(std::shared_ptr<LILNode> newParent);
        virtual void removeFromParentNode();
        const std::vector<std::shared_ptr<LILNode> > & getChildNodes() const;
        virtual void addNode(std::shared_ptr<LILNode> child);
        virtual void removeNode(std::shared_ptr<LILNode> child);
        virtual void prependNode(std::shared_ptr<LILNode> child);
        void setChildNodes(const std::vector<std::shared_ptr<LILNode>> && nodes);
        void clearChildNodes();

        virtual bool isA(ExpressionType otherType) const;
        virtual ExpressionType getExpressionType() const;
        virtual bool isA(InstructionType otherType) const;
        virtual InstructionType getInstructionType() const;
        virtual bool isA(SelectorType otherType) const;
        virtual SelectorType getSelectorType() const;
        virtual bool isA(CombinatorType otherType) const;
        virtual CombinatorType getCombinatorType() const;
        virtual bool isA(FlagFunctionType otherType) const;
        virtual FlagFunctionType getFlagFunctionType() const;
        virtual bool isA(EventType otherType) const;
        virtual EventType getEventType() const;
        virtual bool isA(SelectionType otherType) const;
        virtual SelectionType getSelectionType() const;
        virtual bool isA(FunctionCallType otherType) const;
        virtual FunctionCallType getFunctionCallType() const;
        virtual bool isA(FlowControlType otherType) const;
        virtual FlowControlType getFlowControlType() const;
        virtual bool isA(FlowControlCallType otherType) const;
        virtual FlowControlCallType getFlowControlCallType() const;
        virtual bool isA(TypeType otherType) const;
        virtual TypeType getTypeType() const;

        LILDocument * getDocument() const;
        void setDocument(LILDocument * newDoc);
        LILString getHostProperty() const;
        void setHostProperty(LILString newValue);
        
        void setIsExported(bool value);
        bool getIsExported() const;

        bool hidden;

    protected:
        virtual std::shared_ptr<LILClonable> cloneImpl() const;
        void cloneChildNodes(std::shared_ptr<LILNode> clone) const;
        std::vector<std::shared_ptr<LILNode> > _childNodes;

    private:
        LILDocument * document;
        NodeType nodeType;
        std::weak_ptr<LILNode> _parentNode;
        LILString _hostProperty;
        LILUnitI64 _specificity;
        LILNode::SourceLocation _sourceLocation;
        bool _isExported;
    };
}


#endif
