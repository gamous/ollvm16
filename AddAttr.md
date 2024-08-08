# 添加C++11标准Attribute开关



\clang\include\clang\Basic\Attr.td

末尾追加

```td
def ObfuscationAttr : InheritableAttr {
  let Spellings = [
    CXX11<"","fla",20240808>,
    CXX11<"","bcf",20240808>,
    CXX11<"","split",20240808>,
    CXX11<"","sub",20240808>,
    CXX11<"","nofla",20240808>,
    CXX11<"","nobcf",20240808>,
    CXX11<"","nosplit",20240808>,
    CXX11<"","nosub",20240808>,
    CXX11<"","ibr",20240808>,
    CXX11<"","icall",20240808>,
    CXX11<"","igv",20240808>,
    CXX11<"","noibr",20240808>,
    CXX11<"","noicall",20240808>,
    CXX11<"","noigv",20240808>,
  ];
  let Subjects = SubjectList<[Function],ErrorDiag>;
  let Documentation = [Undocumented];
}
```

此处添加后attr可以进入sema处理流程



\clang\lib\Sema\SemaDeclAttr.cpp

obj.clangSema\SemaDeclAttr.cpp

switch结构中追加对 ParsedAttr的处理，使得attr进入codegen流程，否则无法通过attr合法性断言

```c++
static void
ProcessDeclAttribute(Sema &S, Scope *scope, Decl *D, const ParsedAttr &AL,
                     const Sema::ProcessDeclAttributeOptions &Options) {
//...省略
 switch (AL.getKind()) {
//...省略
  case ParsedAttr::AT_ObfuscationAttr:
    handleSimpleAttribute<ObfuscationAttrAttr>(S, D, AL);     
 }
//...省略
}
```





\clang\lib\CodeGen\CGCall.cpp

obj.clangCodeGen\CGCall.cpp

CodeGen中处理函数序言时将attr添加到funtion中，将attr传递到llvm层

```c++
void CodeGenFunction::EmitFunctionProlog(const CGFunctionInfo &FI,
                                         llvm::Function *Fn,
                                         const FunctionArgList &Args) {
                                         
  if (CurCodeDecl && CurCodeDecl->hasAttr<ObfuscationAttrAttr>())
    // Naked functions don't have prologues.
    for (auto attr : CurCodeDecl->specific_attrs<ObfuscationAttrAttr>()) {
      if (auto spell = attr->getSpelling()) {
        Fn->addFnAttr(llvm::StringRef(spell));
      }
    }
//...省略
}
```



经过上面的流程 attr就透过clang处理进入到llvm ir层 从而可以被pass访问到了

```c++
f->hasFnAttribute(attr)
```

本仓库已经修改好`llvm::toObfuscate`函数实现使其支持attr



c++代码中使用方式

```c++
[[fla,bcf,sub,split,ibr,icall,igv]]
int test() {
    int *testNew = new int[200];
    for(int i=0;i<20;i++)
    	printf("hello world\n");
    return 0;
}
```

```c++
[[nofla,nobcf,nosub,nosplit,noibr,noicall,noigv]]
int test() {
    int *testNew = new int[200];
    for(int i=0;i<20;i++)
    	printf("hello world\n");
    return 0;
}
```

