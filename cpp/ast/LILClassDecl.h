/********************************************************************
 *
 *	  LIL Is a Language
 *
 *	  AUTHORS: Miro Keller
 *
 *	  COPYRIGHT: ©2020-today:  All Rights Reserved
 *
 *	  LICENSE: see LICENSE file
 *
 *	  This file implements object definitions
 *
 ********************************************************************/


#ifndef LILCLASSDECL_H
#define LILCLASSDECL_H


#include "LILTypedNode.h"

namespace LIL
{
	class LILAliasDecl;
	class LILDocumentation;
	
	class LILClassDecl : public LILTypedNode
	{
	public:
		LILClassDecl();
		LILClassDecl(const LILClassDecl &other);
		std::shared_ptr<LILClassDecl> clone() const;
		virtual ~LILClassDecl();
		
		std::shared_ptr<LILNode> getInheritType() const;
		void setInheritType(std::shared_ptr<LILNode> newType);
		
		bool getReceivesInherits() const;
		void setReceivesInherits(bool value);
		bool getReceivesBody() const;
		void setReceivesBody(bool value);
		
		void addField(std::shared_ptr<LILNode> value);
		const std::vector<std::shared_ptr<LILNode>> & getFields() const;
		size_t getIndexOfField(std::shared_ptr<LILNode> field, bool & found) const;
		void addMethod(std::string name, std::shared_ptr<LILNode> value);
		const std::unordered_map<std::string, std::shared_ptr<LILNode>> & getMethods() const;
		LILString getName() const;
		
		std::shared_ptr<LILNode> getFieldNamed(const LILString & name) const;
		std::shared_ptr<LILNode> getMethodNamed(const LILString & name) const;
		std::shared_ptr<LILNode> getAliasNamed(const LILString & name) const;

		bool getIsExtern() const;
		void setIsExtern(bool value);

		void addAlias(std::shared_ptr<LILAliasDecl> value);
		const std::vector<std::shared_ptr<LILAliasDecl>> & getAliases() const;

		void addDoc(std::shared_ptr<LILDocumentation> value);
		const std::vector<std::shared_ptr<LILDocumentation>> & getDocs() const;

		void add(std::shared_ptr<LILNode> node);
		void addOther(std::shared_ptr<LILNode> value);
		const std::vector<std::shared_ptr<LILNode>> & getOther() const;
		void clearOther();
		void setOther(const std::vector<std::shared_ptr<LILNode>> && other);

		bool isTemplate() const;

		const std::vector<std::shared_ptr<LILNode>> & getTmplParams() const;
		void addTmplParam(std::shared_ptr<LILNode> value);
		void setTmplParams(const std::vector<std::shared_ptr<LILNode>> && values);
		void clearTmplParams();

	protected:
		virtual std::shared_ptr<LILClonable> cloneImpl() const;
		
	private:
		bool _isExtern;
		std::shared_ptr<LILNode> _inheritType;
		bool _receivesInherits;
		bool _receivesBody;
		std::vector<std::shared_ptr<LILNode>> _fields;
		std::unordered_map<std::string, std::shared_ptr<LILNode>> _methods;
		std::vector<std::shared_ptr<LILAliasDecl>> _aliases;
		std::vector<std::shared_ptr<LILDocumentation>> _docs;
		std::vector<std::shared_ptr<LILNode>> _other;
		std::vector<std::shared_ptr<LILNode>> _tmplParams;
	};
}

#endif
