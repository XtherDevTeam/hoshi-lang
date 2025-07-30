//
// Created by XIaokang00010 on 2024/10/12.
//

#ifndef HOSHI_LANG_IRLINKER_HPP
#define HOSHI_LANG_IRLINKER_HPP

#include "compiler/ir/IR.h"
#include "compiler/compilerContext.h"
#include <map>
#include <vector>

#define ENTRY_MODULE_ID_CONST 0xe1751a00

namespace yoi {

    class IRLinker {
    public:
        IRLinker();

        /**
        * @brief Links all compiled modules from the context into a single IRObjectFile.
        * @param context The compiler context containing all compiled modules.
        * @param entryModuleId The ID of the main entry module.
        * @return A shared pointer to the final IRObjectFile.
        */
        std::shared_ptr<IRObjectFile> link(const std::shared_ptr<compilerContext>& context, indexT entryModuleId);

    private:
        std::shared_ptr<compilerContext> compilerCtx;
        std::shared_ptr<IRModule> finalModule;
        indexT entryModuleId;

        // Remapping tables: map<old_module_id, map<old_index, new_index>>
        std::map<indexT, std::map<indexT, indexT>> structRemapping;
        std::map<indexT, std::map<indexT, indexT>> interfaceRemapping;
        std::map<indexT, std::map<indexT, indexT>> interfaceImplRemapping;
        std::map<indexT, std::map<indexT, indexT>> globalRemapping;
        std::map<indexT, std::map<indexT, indexT>> functionRemapping;
        std::map<indexT, std::map<indexT, indexT>> stringRemapping;
        yoi::vec<yoi::indexT> globInitializerIndexes;

        /**
        * @brief Mangles a symbol name with its module ID, unless it's the main function in the entry module.
        */
        wstr mangleName(indexT moduleId, const wstr& originalName);

        void linkStringLiterals();
        void linkStructsAndInterfaces();
        void linkInterfaceImplementations();
        void linkGlobals();
        void linkFunctions();
        void createEntryFunction();

        std::shared_ptr<IRValueType> patchType(const std::shared_ptr<IRValueType> &oldType);
        IR patchInstruction(const IR& instr, indexT currentModuleId);
    };

} // namespace yoi

#endif //HOSHI_LANG_IRLINKER_HPP