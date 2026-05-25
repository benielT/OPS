
import pcpp
import sys
from store import ParseError, Location
from typing import List, Any
from dataclasses import dataclass
import logging
@dataclass
class isl_directive:
    dirtoken: Any
    arg_toks: List[Any]
    
    def __str__(self) -> str:
        return f"isl_dir: {self.dirtoken.source}:{self.dirtoken.lineno}, dirtoken: {self.dirtoken}, args: {self.arg_toks}"

    def get_lineno(self):
        return self.dirtoken.lineno
        
    def get_isl_name(self):
        name_tok = self.arg_toks[0]
        return name_tok.value[1:-1]
    
    def get_max_iter_param(self):
        param = ''.join(tok.value for tok in self.arg_toks[1:])
        return param

class Preprocessor(pcpp.Preprocessor):
    def __init__(self, lexer=None):
        super(Preprocessor, self).__init__(lexer)
        self.__iter_parloop_directives = []
        self.line_directive = None
        self.__is_ops_tiled_flag = False
        self.__ops_tile_sizes = [-1,-1,-1]  # x,y,z
        self.__is_ops_tiled_interleave = False

    # preprocessor hook
    def on_comment(self, tok: str) -> bool:
        return True

    # preprocessor hook
    def on_error(self, file: str, line: int, msg: str) -> None:
        loc = Location(file, line, 0)
        raise ParseError("[PREPORC] " + msg, loc)

    # preprocessor hook
    def on_include_not_found(self, is_malformed, is_system_include, curdir, includepath) -> None:
        if is_system_include:
            raise pcpp.OutputDirective(pcpp.Action.IgnoreAndPassThrough)

        super().on_include_not_found(is_malformed, is_system_include, curdir, includepath)
        
    def clean_args(self, precedingtoks):
        cleaned_precedingtoks = []
        
        for tok in precedingtoks:
            if not tok.type == "CPP_WS":
                cleaned_precedingtoks.append(tok)
        return cleaned_precedingtoks
    
    # preprocessor hook      
    def on_directive_unknown(self ,directive, toks, ifpassthru, precedingtoks):
                
        if toks[0].value == "ISL":
            cleaned_args =   self.clean_args(toks[1:])
            
            if len(cleaned_args) < 2:
                self.on_error(directive.source, directive.lineno, f"error in ISL pragma. it got #{len(cleaned_args)} prameters. It should have the name and the total iteration as parameters")
                
            self.__iter_parloop_directives.append(isl_directive(directive,cleaned_args))
            logging.debug("[PREPROC_DEBUG] %s:%d ISL directive: %s" % (directive.source,directive.lineno,''.join(tok.value for tok in toks[1:])))
            return True
        else:
            logging.debug("[PREPROC_DEBUG] %s:%d unknown directive: %s" % (directive.source,directive.lineno,''.join(tok.value for tok in toks)))
            
        # This section is part of original hook
        if directive.value == 'error':
            logging.error("[PREPORC] %s:%d: %s" % (directive.source,directive.lineno,''.join(tok.value for tok in toks)))
            self.on_error(directive.source, directive.lineno, f"error directive wit values: {''.join(tok.value for tok in toks)}")
            self.return_code += 1
            return True
        elif directive.value == 'warning':
            logging.warning("[PREPORC] %s:%d: %s" % (directive.source,directive.lineno,''.join(tok.value for tok in toks)))
            return True
        return None
    
    def get_isl_directives(self) -> Any:
        return self.__iter_parloop_directives
    
    def is_ops_tiled(self)-> bool:
        return self.__is_ops_tiled_flag
    
    def is_ops_tiling_interleaved(self)-> bool:
        if (self.is_ops_tiled and self.__is_ops_tiled_interleave):
            return True
        return False
    
    def extract_macro_value(self, name: str, macro: Any) -> Any:
        if isinstance(macro.value, list) and len(macro.value) > 0:
            token = macro.value[0]
            if token.type == "CPP_INTEGER":
                return int(token.value)
            elif token.type == "CPP_STRING":
                return token.value[1:-1]
            else:
                logging.error(f"[PREPROC] Unsupported macro type for {name}: {token.type}")
                return None
        else:
            logging.error(f"[PREPROC] Macro {name} has no value or unsupported format")
            return None
    
    def post_process(self):
        for name, macro in self.macros.items():
            if name == "OPS_TILING":
                self.__is_ops_tiled_flag = True
            elif name == "OPS_MAXTILESIZE_X": 
                self.__ops_tile_sizes[0] = self.extract_macro_value(name, macro)
                print(f"[PREPROC] X tile size: {self.__ops_tile_sizes[0]}")
                
            elif name == "OPS_MAXTILESIZE_Y":
                self.__ops_tile_sizes[1] = self.extract_macro_value(name, macro)
                print(f"[PREPROC] Y tile size: {self.__ops_tile_sizes[1]}")

            elif name == "OPS_MAXTILESIZE_Z":
                self.__ops_tile_sizes[2] = self.extract_macro_value(name, macro)
                print(f"[PREPROC] Z tile size: {self.__ops_tile_sizes[2]}")
                
            elif name == "OPS_HLS_TILE_INTERLEAVE":
                self.__is_ops_tiled_interleave = True
                print(f"[PREPROC] OPS TILING INTERLEAVE flag set")
                

    def parse(self, input, source) -> None:
        super().parse(input, source)
        self.post_process()
                    
    def get_ops_tile_sizes(self) -> List[int]:
        return self.__ops_tile_sizes
 
    def list_defines(self) -> List[str]:
        return [f"{name}={macro.value}" for name, macro in self.macros.items()]