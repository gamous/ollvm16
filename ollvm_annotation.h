//控制流平坦化
#define OBF_FLA __attribute((__annotate__(("fla"))))
#define OBF_NOFLA __attribute((__annotate__(("nofla"))))
//不透明谓词
#define OBF_BCF __attribute((__annotate__(("bcf"))))
#define OBF_NOBCF __attribute((__annotate__(("nobcf"))))
//指令替换
#define OBF_SUB __attribute((__annotate__(("sub"))))
#define OBF_NOSUB __attribute((__annotate__(("nosub"))))
//块分割
#define OBF_SPLIT __attribute((__annotate__(("split"))))
#define OBF_NOSPLIT __attribute((__annotate__(("nosplit"))))
//调用间接化
#define OBF_ICALL __attribute((__annotate__(("icall"))))
#define OBF_NOICALL __attribute((__annotate__(("noicall"))))
//跳转间接化
#define OBF_IBR __attribute((__annotate__(("ibr"))))
#define OBF_NOIBR __attribute((__annotate__(("noibr"))))
//全局变量访问间接化
#define OBF_IGV __attribute((__annotate__(("igv"))))
#define OBF_NOIGV __attribute((__annotate__(("noigv"))))
//全开
#define OBF_ALL OBF_FLA OBF_BCF OBF_SUB OBF_SPLIT OBF_ICALL OBF_IBR OBF_IGV
#define OBF_NONE OBF_NOFLA OBF_NOBCF OBF_NOSUB OBF_NOSPLIT OBF_NOICALL OBF_NOIBR OBF_NOIGV