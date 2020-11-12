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
 *      This file imports code from other files when using #needs
 *
 ********************************************************************/

#include <glob.h>

#include "LILNeedsImporter.h"
#include "LILAliasDecl.h"
#include "LILClassDecl.h"
#include "LILCodeUnit.h"
#include "LILFunctionDecl.h"
#include "LILInstruction.h"
#include "LILObjectType.h"
#include "LILRootNode.h"
#include "LILStringLiteral.h"
#include "LILTypeDecl.h"
#include "LILVarDecl.h"

using namespace LIL;

LILNeedsImporter::LILNeedsImporter()
{
}

LILNeedsImporter::~LILNeedsImporter()
{
}

void LILNeedsImporter::initializeVisit()
{
    if (this->getVerbose()) {
        std::cerr << "\n\n";
        std::cerr << "============================\n";
        std::cerr << "===== #NEEDS IMPORTING =====\n";
        std::cerr << "============================\n\n";
    }
}

void LILNeedsImporter::visit(LILNode *node)
{
    
}

void LILNeedsImporter::performVisit(std::shared_ptr<LILRootNode> rootNode)
{
    this->setRootNode(rootNode);
    
    std::vector<std::shared_ptr<LILNode>> nodes = rootNode->getNodes();
    std::vector<std::shared_ptr<LILNode>> newNodes;
    std::vector<std::shared_ptr<LILNode>> remainingNodes;
    for (auto node : nodes) {
        if (node->isA(InstructionTypeNeeds)) {
            auto instr = std::static_pointer_cast<LILInstruction>(node);
            auto arg = instr->getArgument();
            if (!arg->isA(NodeTypeStringLiteral)) {
                continue;
            }
            LILString argStr = std::static_pointer_cast<LILStringLiteral>(arg)->getValue().stripQuotes();
            auto paths = this->_resolveFilePaths(argStr);
            for (auto path : paths) {
                if (this->getVerbose()) {
                    std::cerr << "Start of file " << path.data() << "\n\n========================================\n\n";
                }
                
                std::unique_ptr<LILCodeUnit> codeUnit = std::make_unique<LILCodeUnit>();
                codeUnit->setFile(path);
                LILString dir = this->_getDir(path);
                codeUnit->setDir(dir);
                std::ifstream file(path.data(), std::ios::in);
                if (file.fail()) {
                    std::cerr << "\nERROR: Failed to read the file "+path.data()+"\n\n";
                    continue;
                }
                std::stringstream buffer;
                buffer << file.rdbuf();
                LILString lilStr(buffer.str());
                codeUnit->setSource(lilStr);
                codeUnit->setDebugAST(this->getDebugAST());
                codeUnit->setVerbose(this->getVerbose());
                codeUnit->run();
                auto rootNode = codeUnit->getRootNode();
                this->_getNodesForImport(&newNodes, rootNode);
                
                if (this->getVerbose()) {
                    std::cerr << "\nEnd of file " << path.data() << "\n\n========================================\n\n";
                }
            }
        } else {
            remainingNodes.push_back(node);
        }
    }
    for (auto newNode : newNodes) {
        switch (newNode->getNodeType()) {
            case NodeTypeVarDecl:
            {
                auto vd = std::static_pointer_cast<LILVarDecl>(newNode);
                
                //set local variable
                rootNode->setLocalVariable(vd->getName(), vd);
                
                if (vd->getIsExtern()) {
                    rootNode->addNode(vd);
                } else {
                    auto initVal = vd->getInitVal();
                    if (initVal) {
                        if (initVal->isA(NodeTypeFunctionDecl)) {
                            rootNode->addNode(vd);
                        } else {
                            rootNode->getMainFn()->addEvaluable(vd);
                        }
                    } else {
                        rootNode->getMainFn()->addEvaluable(vd);
                    }
                }
                break;
            }
            case NodeTypeAliasDecl:
            {
                auto ad = std::static_pointer_cast<LILAliasDecl>(newNode);
                rootNode->addNode(ad);
                rootNode->addAlias(ad);
                break;
            }
            case NodeTypeTypeDecl:
            {
                auto td = std::static_pointer_cast<LILTypeDecl>(newNode);
                rootNode->addNode(td);
                rootNode->addType(td);
                break;
            }
            case NodeTypeClassDecl:
            {
                auto cd = std::static_pointer_cast<LILClassDecl>(newNode);
                rootNode->addNode(cd);
                rootNode->addClass(cd);
                break;
            }
                
            case NodeTypeInstruction:
            {
                auto instr = std::static_pointer_cast<LILInstruction>(newNode);
                switch (instr->getInstructionType()) {
                    case InstructionTypeNeeds:
                    {
                        rootNode->addNode(instr);
                        rootNode->addDependency(instr);
                        break;
                    }
                        
                    default:
                        break;
                }
                break;
            }
                
            default:
                rootNode->getMainFn()->addEvaluable(newNode);
                break;
        }
    }
    newNodes.insert(newNodes.begin() + newNodes.size(), remainingNodes.begin(), remainingNodes.end());
    rootNode->setChildNodes(newNodes);
}

void LILNeedsImporter::setDir(LILString dir)
{
    this->_dir = dir;
}

LILString LILNeedsImporter::getDir() const
{
    return this->_dir;
}

bool LILNeedsImporter::getDebugAST() const
{
    return this->_debugAST;
}

void LILNeedsImporter::setDebugAST(bool value)
{
    this->_debugAST = value;
}

std::vector<LILString> LILNeedsImporter::_resolveFilePaths(LILString argStr) const
{
    std::vector<LILString> ret;
    std::string dir = this->getDir().data() + "/";
    std::string arg = argStr.data();
    if (arg.find("*") == std::string::npos) {
        ret.push_back(dir+argStr.data());
    } else {
        std::vector<std::string> paths = this->_glob(dir+argStr.data());
        for (auto path : paths) {
            ret.push_back(path);
        }
    }
    return ret;
}

//FIXME: needs windows counterpart
std::vector<std::string> LILNeedsImporter::_glob(const std::string& pattern) const
{
    std::vector<std::string> filenames;
    
    // glob struct resides on the stack
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));
    
    // do the glob operation
    int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    if(return_value != 0) {
        globfree(&glob_result);
        std::cerr << "glob() failed with return_value " << return_value << std::endl;
        return filenames;
    }

    // collect all the filenames into a std::list<std::string>
    
    for(size_t i = 0; i < glob_result.gl_pathc; ++i) {
        filenames.push_back(std::string(glob_result.gl_pathv[i]));
    }
    
    // cleanup
    globfree(&glob_result);
    
    // done
    return filenames;
}

void LILNeedsImporter::_getNodesForImport(std::vector<std::shared_ptr<LILNode>> * newNodes, std::shared_ptr<LILRootNode> rootNode) const
{
    for (auto node : rootNode->getNodes()) {
        switch (node->getNodeType()) {
            case NodeTypeVarDecl:
            {
                auto vd = std::static_pointer_cast<LILVarDecl>(node);
                auto ty = vd->getType();
                if (ty && ty->isA(TypeTypeFunction)) {
                    auto newVd = std::make_shared<LILVarDecl>();
                    newVd->setIsExtern(true);
                    newVd->setName(vd->getName());
                    newVd->setType(ty->clone());
                    newNodes->push_back(newVd);
                }
                break;
            }
                
            case NodeTypeClassDecl:
            {
                auto cd = std::static_pointer_cast<LILClassDecl>(node);
                
                auto newCd = std::make_shared<LILClassDecl>();
                newCd->setIsExtern(true);
                auto newTy = std::make_shared<LILObjectType>();
                newTy->setTypeType(TypeTypeObject);
                newTy->setName(cd->getName());
                newCd->setType(newTy);
                
                auto fields = cd->getFields();
                for (auto field : fields) {
                    if (!field->isA(NodeTypeVarDecl)) {
                        continue;
                    }
                    auto fldVd = std::static_pointer_cast<LILVarDecl>(field);
                    auto newVd = std::make_shared<LILVarDecl>();
                    auto fldTy = field->getType();
                    if (!fldTy) {
                        continue;
                    }
                    newVd->setType(fldTy->clone());
                    newVd->setName(fldVd->getName());
                    newCd->addField(newVd);
                }
                
                auto methods = cd->getMethods();
                for (auto method : methods) {
                    if (!method->isA(NodeTypeVarDecl)) {
                        continue;
                    }
                    auto metVd = std::static_pointer_cast<LILVarDecl>(method);
                    auto newVd = std::make_shared<LILVarDecl>();
                    auto metTy = method->getType();
                    if (!metTy) {
                        continue;
                    }
                    newVd->setType(metTy->clone());
                    newVd->setName(metVd->getName());
                    newCd->addMethod(newVd);
                }
                
                newNodes->push_back(newCd);
                break;
            }
                
            case NodeTypeAliasDecl:
            case NodeTypeTypeDecl:
            {
                newNodes->push_back(node->clone());
                break;
            }

            default:
                break;
        }
    }
}

LILString LILNeedsImporter::_getDir(LILString path) const
{
    std::string dir = path.data();
    size_t i = 0;
    size_t pos = 0;
    bool found = false;
    for (std::string::iterator it = dir.begin(); it != dir.end(); ++it) {
        char cc = *it;
        if (cc == '/' || cc == '\\') {
            found = true;
            pos = i;
        }
        i += 1;
    }
    if (found) {
        return dir.substr(0, pos);
    } else {
        return "";
    }
}
