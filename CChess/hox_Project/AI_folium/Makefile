# Your project's name.
PROGRAM = AI_folium

# Libraries specific flags.
BOOST_CXXFLAGS =

# Common flags
CXX         = g++
CXXFLAGS    = $(BOOST_CXXFLAGS) -Wall -Werror
LDLIBS      = $(ST_LDLIBS) -lpthread
LDFLAGS     =
DEBUGFLAGS  = -g

# FOLIUM AI Engine source
FOLIUM_SRC := \
	bitmap_data.cpp \
	folEngine.cpp \
	engine_data.cpp \
	generator.cpp \
	hash.cpp \
	history_data.cpp \
	move.cpp \
	folSearch.cpp \
	xq.cpp \
	xq_data.cpp \
	folHOXEngine.cpp

MAIN_SRC := \
	TcpLib.cpp \
	hoxCommon.cpp \
	hoxCommand.cpp \
	AIPlayer.cpp \
	main.cpp

# Define our sources and object files
SOURCES := \
	$(FOLIUM_SRC) \
	$(MAIN_SRC)

OBJECTS := $(SOURCES:.cpp=.o)

.cpp.o :
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c  -o $@ $<

all: $(PROGRAM)

prepare:
	cp -v ../folium/* .

$(PROGRAM): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJECTS) $(LDLIBS)

depend: $(SOURCES)
	makedepend $(CXXFLAGS) $(SOURCES)

clean:
	ls ../folium/*.h | cut -c11- | xargs rm -f
	rm -rf $(PROGRAM) $(OBJECTS) *.bak $(FOLIUM_SRC)

zip:
	zip $(PROGRAM) $(MAIN_SRC) ../folium/* *.h Makefile *.sh

# DO NOT DELETE

bitmap_data.o: defines.h int.h bitmap.h /usr/include/string.h
bitmap_data.o: /usr/include/features.h /usr/include/sys/cdefs.h
bitmap_data.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
bitmap_data.o: /usr/include/gnu/stubs-64.h xq_data.h
folEngine.o: folEngine.h defines.h int.h movelist.h move.h xq_data.h xq.h
folEngine.o: bitmap.h /usr/include/string.h /usr/include/features.h
folEngine.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
folEngine.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h history.h
folEngine.o: hash.h engine_data.h
engine_data.o: defines.h int.h engine_data.h
generator.o: generator.h defines.h int.h movelist.h move.h xq.h bitmap.h
generator.o: /usr/include/string.h /usr/include/features.h
generator.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
generator.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h xq_data.h
generator.o: killer.h history.h
hash.o: hash.h defines.h int.h xq.h move.h bitmap.h /usr/include/string.h
hash.o: /usr/include/features.h /usr/include/sys/cdefs.h
hash.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
hash.o: /usr/include/gnu/stubs-64.h xq_data.h
history_data.o: defines.h int.h history.h move.h
move.o: move.h int.h xq_data.h defines.h
folSearch.o: folEngine.h defines.h int.h movelist.h move.h xq_data.h xq.h
folSearch.o: bitmap.h /usr/include/string.h /usr/include/features.h
folSearch.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
folSearch.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h history.h
folSearch.o: hash.h engine_data.h killer.h generator.h
xq.o: xq.h defines.h int.h move.h bitmap.h /usr/include/string.h
xq.o: /usr/include/features.h /usr/include/sys/cdefs.h
xq.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
xq.o: /usr/include/gnu/stubs-64.h xq_data.h
xq_data.o: defines.h int.h xq_data.h
folHOXEngine.o: folHOXEngine.h folEngine.h defines.h int.h movelist.h move.h
folHOXEngine.o: xq_data.h xq.h bitmap.h /usr/include/string.h
folHOXEngine.o: /usr/include/features.h /usr/include/sys/cdefs.h
folHOXEngine.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
folHOXEngine.o: /usr/include/gnu/stubs-64.h history.h hash.h engine_data.h
TcpLib.o: /usr/include/netdb.h /usr/include/features.h
TcpLib.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
TcpLib.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
TcpLib.o: /usr/include/netinet/in.h /usr/include/stdint.h
TcpLib.o: /usr/include/bits/wchar.h /usr/include/sys/socket.h
TcpLib.o: /usr/include/sys/uio.h /usr/include/sys/types.h
TcpLib.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
TcpLib.o: /usr/include/time.h /usr/include/endian.h
TcpLib.o: /usr/include/bits/endian.h /usr/include/sys/select.h
TcpLib.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
TcpLib.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
TcpLib.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/uio.h
TcpLib.o: /usr/include/bits/socket.h /usr/include/limits.h
TcpLib.o: /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h
TcpLib.o: /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h
TcpLib.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
TcpLib.o: /usr/include/asm-x86_64/socket.h /usr/include/asm/sockios.h
TcpLib.o: /usr/include/asm-x86_64/sockios.h /usr/include/bits/in.h
TcpLib.o: /usr/include/bits/byteswap.h /usr/include/rpc/netdb.h
TcpLib.o: /usr/include/bits/netdb.h /usr/include/unistd.h
TcpLib.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
TcpLib.o: /usr/include/getopt.h /usr/include/errno.h
TcpLib.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
TcpLib.o: /usr/include/asm/errno.h /usr/include/asm-x86_64/errno.h
TcpLib.o: /usr/include/asm-generic/errno.h
TcpLib.o: /usr/include/asm-generic/errno-base.h TcpLib.h
hoxCommon.o: hoxCommon.h /usr/include/boost/tokenizer.hpp
hoxCommon.o: /usr/include/boost/token_iterator.hpp
hoxCommon.o: /usr/include/boost/assert.hpp /usr/include/assert.h
hoxCommon.o: /usr/include/features.h /usr/include/sys/cdefs.h
hoxCommon.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
hoxCommon.o: /usr/include/gnu/stubs-64.h
hoxCommon.o: /usr/include/boost/iterator/iterator_adaptor.hpp
hoxCommon.o: /usr/include/boost/static_assert.hpp
hoxCommon.o: /usr/include/boost/config.hpp /usr/include/boost/config/user.hpp
hoxCommon.o: /usr/include/boost/config/select_compiler_config.hpp
hoxCommon.o: /usr/include/boost/config/compiler/gcc.hpp
hoxCommon.o: /usr/include/boost/config/select_stdlib_config.hpp
hoxCommon.o: /usr/include/boost/config/no_tr1/utility.hpp
hoxCommon.o: /usr/include/boost/config/select_platform_config.hpp
hoxCommon.o: /usr/include/boost/config/posix_features.hpp
hoxCommon.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
hoxCommon.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
hoxCommon.o: /usr/include/bits/confname.h /usr/include/getopt.h
hoxCommon.o: /usr/include/boost/config/suffix.hpp
hoxCommon.o: /usr/include/boost/detail/workaround.hpp
hoxCommon.o: /usr/include/boost/iterator.hpp /usr/include/boost/config.hpp
hoxCommon.o: /usr/include/boost/detail/iterator.hpp
hoxCommon.o: /usr/include/boost/iterator/iterator_categories.hpp
hoxCommon.o: /usr/include/boost/iterator/detail/config_def.hpp
hoxCommon.o: /usr/include/boost/mpl/eval_if.hpp /usr/include/boost/mpl/if.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/value_wknd.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/static_cast.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/workaround.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/integral.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/msvc.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/eti.hpp
hoxCommon.o: /usr/include/boost/mpl/int.hpp
hoxCommon.o: /usr/include/boost/mpl/int_fwd.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/adl_barrier.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/adl.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/intel.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/gcc.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/nttp_decl.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/nttp.hpp
hoxCommon.o: /usr/include/boost/preprocessor/cat.hpp
hoxCommon.o: /usr/include/boost/preprocessor/config/config.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/integral_wrapper.hpp
hoxCommon.o: /usr/include/boost/mpl/integral_c_tag.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/static_constant.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/na_spec.hpp
hoxCommon.o: /usr/include/boost/mpl/lambda_fwd.hpp
hoxCommon.o: /usr/include/boost/mpl/void_fwd.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/na.hpp
hoxCommon.o: /usr/include/boost/mpl/bool.hpp
hoxCommon.o: /usr/include/boost/mpl/bool_fwd.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/na_fwd.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/ctps.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/lambda.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/ttp.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/lambda_arity_param.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/template_arity_fwd.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/arity.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/dtp.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/preprocessor/params.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/preprocessor.hpp
hoxCommon.o: /usr/include/boost/preprocessor/comma_if.hpp
hoxCommon.o: /usr/include/boost/preprocessor/punctuation/comma_if.hpp
hoxCommon.o: /usr/include/boost/preprocessor/control/if.hpp
hoxCommon.o: /usr/include/boost/preprocessor/control/iif.hpp
hoxCommon.o: /usr/include/boost/preprocessor/logical/bool.hpp
hoxCommon.o: /usr/include/boost/preprocessor/facilities/empty.hpp
hoxCommon.o: /usr/include/boost/preprocessor/punctuation/comma.hpp
hoxCommon.o: /usr/include/boost/preprocessor/repeat.hpp
hoxCommon.o: /usr/include/boost/preprocessor/repetition/repeat.hpp
hoxCommon.o: /usr/include/boost/preprocessor/debug/error.hpp
hoxCommon.o: /usr/include/boost/preprocessor/detail/auto_rec.hpp
hoxCommon.o: /usr/include/boost/preprocessor/tuple/eat.hpp
hoxCommon.o: /usr/include/boost/preprocessor/inc.hpp
hoxCommon.o: /usr/include/boost/preprocessor/arithmetic/inc.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/preprocessor/enum.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/preprocessor/def_params_tail.hpp
hoxCommon.o: /usr/include/boost/mpl/limits/arity.hpp
hoxCommon.o: /usr/include/boost/preprocessor/logical/and.hpp
hoxCommon.o: /usr/include/boost/preprocessor/logical/bitand.hpp
hoxCommon.o: /usr/include/boost/preprocessor/identity.hpp
hoxCommon.o: /usr/include/boost/preprocessor/facilities/identity.hpp
hoxCommon.o: /usr/include/boost/preprocessor/empty.hpp
hoxCommon.o: /usr/include/boost/preprocessor/arithmetic/add.hpp
hoxCommon.o: /usr/include/boost/preprocessor/arithmetic/dec.hpp
hoxCommon.o: /usr/include/boost/preprocessor/control/while.hpp
hoxCommon.o: /usr/include/boost/preprocessor/list/fold_left.hpp
hoxCommon.o: /usr/include/boost/preprocessor/list/detail/fold_left.hpp
hoxCommon.o: /usr/include/boost/preprocessor/control/expr_iif.hpp
hoxCommon.o: /usr/include/boost/preprocessor/list/adt.hpp
hoxCommon.o: /usr/include/boost/preprocessor/detail/is_binary.hpp
hoxCommon.o: /usr/include/boost/preprocessor/detail/check.hpp
hoxCommon.o: /usr/include/boost/preprocessor/logical/compl.hpp
hoxCommon.o: /usr/include/boost/preprocessor/list/fold_right.hpp
hoxCommon.o: /usr/include/boost/preprocessor/list/detail/fold_right.hpp
hoxCommon.o: /usr/include/boost/preprocessor/list/reverse.hpp
hoxCommon.o: /usr/include/boost/preprocessor/control/detail/while.hpp
hoxCommon.o: /usr/include/boost/preprocessor/tuple/elem.hpp
hoxCommon.o: /usr/include/boost/preprocessor/arithmetic/sub.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/overload_resolution.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/lambda_support.hpp
hoxCommon.o: /usr/include/boost/mpl/identity.hpp
hoxCommon.o: /usr/include/boost/mpl/placeholders.hpp
hoxCommon.o: /usr/include/boost/mpl/arg.hpp
hoxCommon.o: /usr/include/boost/mpl/arg_fwd.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/na_assert.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/arity_spec.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/arg_typedef.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/use_preprocessed.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/compiler.hpp
hoxCommon.o: /usr/include/boost/preprocessor/stringize.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_convertible.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/yes_no_type.hpp
hoxCommon.o: /usr/include/boost/type_traits/config.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_array.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/bool_trait_def.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/template_arity_spec.hpp
hoxCommon.o: /usr/include/boost/type_traits/integral_constant.hpp
hoxCommon.o: /usr/include/boost/mpl/integral_c.hpp
hoxCommon.o: /usr/include/boost/mpl/integral_c_fwd.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/bool_trait_undef.hpp
hoxCommon.o: /usr/include/boost/type_traits/add_reference.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_reference.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/type_trait_def.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/type_trait_undef.hpp
hoxCommon.o: /usr/include/boost/type_traits/ice.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/ice_or.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/ice_and.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/ice_not.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/ice_eq.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_arithmetic.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_integral.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_float.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_void.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_abstract.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_class.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_union.hpp
hoxCommon.o: /usr/include/boost/type_traits/remove_cv.hpp
hoxCommon.o: /usr/include/boost/type_traits/broken_compiler_spec.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/cv_traits_impl.hpp
hoxCommon.o: /usr/include/boost/type_traits/intrinsics.hpp
hoxCommon.o: /usr/include/boost/iterator/detail/config_undef.hpp
hoxCommon.o: /usr/include/boost/iterator/iterator_facade.hpp
hoxCommon.o: /usr/include/boost/iterator/interoperable.hpp
hoxCommon.o: /usr/include/boost/mpl/or.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/nested_type_wknd.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommon.o: /usr/include/boost/iterator/iterator_traits.hpp
hoxCommon.o: /usr/include/boost/iterator/detail/facade_iterator_category.hpp
hoxCommon.o: /usr/include/boost/mpl/and.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_same.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_const.hpp
hoxCommon.o: /usr/include/boost/detail/indirect_traits.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_function.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/false_result.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/is_function_ptr_helper.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_pointer.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_member_pointer.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_member_function_pointer.hpp
hoxCommon.o: /usr/include/boost/type_traits/detail/is_mem_fun_pointer_impl.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_volatile.hpp
hoxCommon.o: /usr/include/boost/type_traits/remove_reference.hpp
hoxCommon.o: /usr/include/boost/type_traits/remove_pointer.hpp
hoxCommon.o: /usr/include/boost/mpl/not.hpp
hoxCommon.o: /usr/include/boost/iterator/detail/enable_if.hpp
hoxCommon.o: /usr/include/boost/implicit_cast.hpp
hoxCommon.o: /usr/include/boost/type_traits/add_const.hpp
hoxCommon.o: /usr/include/boost/type_traits/add_pointer.hpp
hoxCommon.o: /usr/include/boost/type_traits/remove_const.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_pod.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_scalar.hpp
hoxCommon.o: /usr/include/boost/type_traits/is_enum.hpp
hoxCommon.o: /usr/include/boost/mpl/always.hpp
hoxCommon.o: /usr/include/boost/mpl/apply.hpp
hoxCommon.o: /usr/include/boost/mpl/apply_fwd.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommon.o: /usr/include/boost/mpl/apply_wrap.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/has_apply.hpp
hoxCommon.o: /usr/include/boost/mpl/has_xxx.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/type_wrapper.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/yes_no.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/arrays.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/has_xxx.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/msvc_typename.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/has_apply.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/msvc_never_true.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommon.o: /usr/include/boost/mpl/lambda.hpp
hoxCommon.o: /usr/include/boost/mpl/bind.hpp
hoxCommon.o: /usr/include/boost/mpl/bind_fwd.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/config/bind.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommon.o: /usr/include/boost/mpl/next.hpp
hoxCommon.o: /usr/include/boost/mpl/next_prior.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/common_name_wknd.hpp
hoxCommon.o: /usr/include/boost/mpl/protect.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/full_lambda.hpp
hoxCommon.o: /usr/include/boost/mpl/quote.hpp /usr/include/boost/mpl/void.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/has_type.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/template_arity.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommon.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommon.o: /usr/include/boost/iterator/detail/minimum_category.hpp
hoxCommon.o: /usr/include/boost/token_functions.hpp
hoxCommand.o: hoxCommand.h hoxCommon.h /usr/include/boost/tokenizer.hpp
hoxCommand.o: /usr/include/boost/token_iterator.hpp
hoxCommand.o: /usr/include/boost/assert.hpp /usr/include/assert.h
hoxCommand.o: /usr/include/features.h /usr/include/sys/cdefs.h
hoxCommand.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
hoxCommand.o: /usr/include/gnu/stubs-64.h
hoxCommand.o: /usr/include/boost/iterator/iterator_adaptor.hpp
hoxCommand.o: /usr/include/boost/static_assert.hpp
hoxCommand.o: /usr/include/boost/config.hpp
hoxCommand.o: /usr/include/boost/config/user.hpp
hoxCommand.o: /usr/include/boost/config/select_compiler_config.hpp
hoxCommand.o: /usr/include/boost/config/compiler/gcc.hpp
hoxCommand.o: /usr/include/boost/config/select_stdlib_config.hpp
hoxCommand.o: /usr/include/boost/config/no_tr1/utility.hpp
hoxCommand.o: /usr/include/boost/config/select_platform_config.hpp
hoxCommand.o: /usr/include/boost/config/posix_features.hpp
hoxCommand.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
hoxCommand.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
hoxCommand.o: /usr/include/bits/confname.h /usr/include/getopt.h
hoxCommand.o: /usr/include/boost/config/suffix.hpp
hoxCommand.o: /usr/include/boost/detail/workaround.hpp
hoxCommand.o: /usr/include/boost/iterator.hpp /usr/include/boost/config.hpp
hoxCommand.o: /usr/include/boost/detail/iterator.hpp
hoxCommand.o: /usr/include/boost/iterator/iterator_categories.hpp
hoxCommand.o: /usr/include/boost/iterator/detail/config_def.hpp
hoxCommand.o: /usr/include/boost/mpl/eval_if.hpp
hoxCommand.o: /usr/include/boost/mpl/if.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/value_wknd.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/static_cast.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/workaround.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/integral.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/msvc.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/eti.hpp
hoxCommand.o: /usr/include/boost/mpl/int.hpp
hoxCommand.o: /usr/include/boost/mpl/int_fwd.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/adl_barrier.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/adl.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/intel.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/gcc.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/nttp_decl.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/nttp.hpp
hoxCommand.o: /usr/include/boost/preprocessor/cat.hpp
hoxCommand.o: /usr/include/boost/preprocessor/config/config.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/integral_wrapper.hpp
hoxCommand.o: /usr/include/boost/mpl/integral_c_tag.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/static_constant.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/na_spec.hpp
hoxCommand.o: /usr/include/boost/mpl/lambda_fwd.hpp
hoxCommand.o: /usr/include/boost/mpl/void_fwd.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/na.hpp
hoxCommand.o: /usr/include/boost/mpl/bool.hpp
hoxCommand.o: /usr/include/boost/mpl/bool_fwd.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/na_fwd.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/ctps.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/lambda.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/ttp.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/lambda_arity_param.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/template_arity_fwd.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/arity.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/dtp.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/preprocessor/params.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/preprocessor.hpp
hoxCommand.o: /usr/include/boost/preprocessor/comma_if.hpp
hoxCommand.o: /usr/include/boost/preprocessor/punctuation/comma_if.hpp
hoxCommand.o: /usr/include/boost/preprocessor/control/if.hpp
hoxCommand.o: /usr/include/boost/preprocessor/control/iif.hpp
hoxCommand.o: /usr/include/boost/preprocessor/logical/bool.hpp
hoxCommand.o: /usr/include/boost/preprocessor/facilities/empty.hpp
hoxCommand.o: /usr/include/boost/preprocessor/punctuation/comma.hpp
hoxCommand.o: /usr/include/boost/preprocessor/repeat.hpp
hoxCommand.o: /usr/include/boost/preprocessor/repetition/repeat.hpp
hoxCommand.o: /usr/include/boost/preprocessor/debug/error.hpp
hoxCommand.o: /usr/include/boost/preprocessor/detail/auto_rec.hpp
hoxCommand.o: /usr/include/boost/preprocessor/tuple/eat.hpp
hoxCommand.o: /usr/include/boost/preprocessor/inc.hpp
hoxCommand.o: /usr/include/boost/preprocessor/arithmetic/inc.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/preprocessor/enum.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/preprocessor/def_params_tail.hpp
hoxCommand.o: /usr/include/boost/mpl/limits/arity.hpp
hoxCommand.o: /usr/include/boost/preprocessor/logical/and.hpp
hoxCommand.o: /usr/include/boost/preprocessor/logical/bitand.hpp
hoxCommand.o: /usr/include/boost/preprocessor/identity.hpp
hoxCommand.o: /usr/include/boost/preprocessor/facilities/identity.hpp
hoxCommand.o: /usr/include/boost/preprocessor/empty.hpp
hoxCommand.o: /usr/include/boost/preprocessor/arithmetic/add.hpp
hoxCommand.o: /usr/include/boost/preprocessor/arithmetic/dec.hpp
hoxCommand.o: /usr/include/boost/preprocessor/control/while.hpp
hoxCommand.o: /usr/include/boost/preprocessor/list/fold_left.hpp
hoxCommand.o: /usr/include/boost/preprocessor/list/detail/fold_left.hpp
hoxCommand.o: /usr/include/boost/preprocessor/control/expr_iif.hpp
hoxCommand.o: /usr/include/boost/preprocessor/list/adt.hpp
hoxCommand.o: /usr/include/boost/preprocessor/detail/is_binary.hpp
hoxCommand.o: /usr/include/boost/preprocessor/detail/check.hpp
hoxCommand.o: /usr/include/boost/preprocessor/logical/compl.hpp
hoxCommand.o: /usr/include/boost/preprocessor/list/fold_right.hpp
hoxCommand.o: /usr/include/boost/preprocessor/list/detail/fold_right.hpp
hoxCommand.o: /usr/include/boost/preprocessor/list/reverse.hpp
hoxCommand.o: /usr/include/boost/preprocessor/control/detail/while.hpp
hoxCommand.o: /usr/include/boost/preprocessor/tuple/elem.hpp
hoxCommand.o: /usr/include/boost/preprocessor/arithmetic/sub.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/overload_resolution.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/lambda_support.hpp
hoxCommand.o: /usr/include/boost/mpl/identity.hpp
hoxCommand.o: /usr/include/boost/mpl/placeholders.hpp
hoxCommand.o: /usr/include/boost/mpl/arg.hpp
hoxCommand.o: /usr/include/boost/mpl/arg_fwd.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/na_assert.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/arity_spec.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/arg_typedef.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/use_preprocessed.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/compiler.hpp
hoxCommand.o: /usr/include/boost/preprocessor/stringize.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_convertible.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/yes_no_type.hpp
hoxCommand.o: /usr/include/boost/type_traits/config.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_array.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/bool_trait_def.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/template_arity_spec.hpp
hoxCommand.o: /usr/include/boost/type_traits/integral_constant.hpp
hoxCommand.o: /usr/include/boost/mpl/integral_c.hpp
hoxCommand.o: /usr/include/boost/mpl/integral_c_fwd.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/bool_trait_undef.hpp
hoxCommand.o: /usr/include/boost/type_traits/add_reference.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_reference.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/type_trait_def.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/type_trait_undef.hpp
hoxCommand.o: /usr/include/boost/type_traits/ice.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/ice_or.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/ice_and.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/ice_not.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/ice_eq.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_arithmetic.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_integral.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_float.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_void.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_abstract.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_class.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_union.hpp
hoxCommand.o: /usr/include/boost/type_traits/remove_cv.hpp
hoxCommand.o: /usr/include/boost/type_traits/broken_compiler_spec.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/cv_traits_impl.hpp
hoxCommand.o: /usr/include/boost/type_traits/intrinsics.hpp
hoxCommand.o: /usr/include/boost/iterator/detail/config_undef.hpp
hoxCommand.o: /usr/include/boost/iterator/iterator_facade.hpp
hoxCommand.o: /usr/include/boost/iterator/interoperable.hpp
hoxCommand.o: /usr/include/boost/mpl/or.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/nested_type_wknd.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommand.o: /usr/include/boost/iterator/iterator_traits.hpp
hoxCommand.o: /usr/include/boost/iterator/detail/facade_iterator_category.hpp
hoxCommand.o: /usr/include/boost/mpl/and.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_same.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_const.hpp
hoxCommand.o: /usr/include/boost/detail/indirect_traits.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_function.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/false_result.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/is_function_ptr_helper.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_pointer.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_member_pointer.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_member_function_pointer.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/is_mem_fun_pointer_impl.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_volatile.hpp
hoxCommand.o: /usr/include/boost/type_traits/remove_reference.hpp
hoxCommand.o: /usr/include/boost/type_traits/remove_pointer.hpp
hoxCommand.o: /usr/include/boost/mpl/not.hpp
hoxCommand.o: /usr/include/boost/iterator/detail/enable_if.hpp
hoxCommand.o: /usr/include/boost/implicit_cast.hpp
hoxCommand.o: /usr/include/boost/type_traits/add_const.hpp
hoxCommand.o: /usr/include/boost/type_traits/add_pointer.hpp
hoxCommand.o: /usr/include/boost/type_traits/remove_const.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_pod.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_scalar.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_enum.hpp
hoxCommand.o: /usr/include/boost/mpl/always.hpp
hoxCommand.o: /usr/include/boost/mpl/apply.hpp
hoxCommand.o: /usr/include/boost/mpl/apply_fwd.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommand.o: /usr/include/boost/mpl/apply_wrap.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/has_apply.hpp
hoxCommand.o: /usr/include/boost/mpl/has_xxx.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/type_wrapper.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/yes_no.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/arrays.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/has_xxx.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/msvc_typename.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/has_apply.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/msvc_never_true.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommand.o: /usr/include/boost/mpl/lambda.hpp
hoxCommand.o: /usr/include/boost/mpl/bind.hpp
hoxCommand.o: /usr/include/boost/mpl/bind_fwd.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/config/bind.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommand.o: /usr/include/boost/mpl/next.hpp
hoxCommand.o: /usr/include/boost/mpl/next_prior.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/common_name_wknd.hpp
hoxCommand.o: /usr/include/boost/mpl/protect.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/full_lambda.hpp
hoxCommand.o: /usr/include/boost/mpl/quote.hpp
hoxCommand.o: /usr/include/boost/mpl/void.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/has_type.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/template_arity.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommand.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
hoxCommand.o: /usr/include/boost/iterator/detail/minimum_category.hpp
hoxCommand.o: /usr/include/boost/token_functions.hpp
hoxCommand.o: /usr/include/boost/algorithm/string.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/std_containers_traits.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/std/string_traits.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/yes_no_type.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/sequence_traits.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/std/list_traits.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/trim.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/config.hpp
hoxCommand.o: /usr/include/boost/range/begin.hpp
hoxCommand.o: /usr/include/boost/range/config.hpp
hoxCommand.o: /usr/include/boost/range/iterator.hpp
hoxCommand.o: /usr/include/boost/range/const_iterator.hpp
hoxCommand.o: /usr/include/boost/range/end.hpp
hoxCommand.o: /usr/include/boost/range/detail/implementation_help.hpp
hoxCommand.o: /usr/include/boost/range/detail/common.hpp
hoxCommand.o: /usr/include/boost/range/detail/sfinae.hpp
hoxCommand.o: /usr/include/string.h /usr/include/wchar.h /usr/include/stdio.h
hoxCommand.o: /usr/include/bits/wchar.h
hoxCommand.o: /usr/include/boost/algorithm/string/detail/trim.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/classification.hpp
hoxCommand.o: /usr/include/boost/range/value_type.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/detail/classification.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/predicate_facade.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/case_conv.hpp
hoxCommand.o: /usr/include/boost/iterator/transform_iterator.hpp
hoxCommand.o: /usr/include/boost/function.hpp
hoxCommand.o: /usr/include/boost/preprocessor/iterate.hpp
hoxCommand.o: /usr/include/boost/preprocessor/iteration/iterate.hpp
hoxCommand.o: /usr/include/boost/preprocessor/array/elem.hpp
hoxCommand.o: /usr/include/boost/preprocessor/array/data.hpp
hoxCommand.o: /usr/include/boost/preprocessor/array/size.hpp
hoxCommand.o: /usr/include/boost/preprocessor/slot/slot.hpp
hoxCommand.o: /usr/include/boost/preprocessor/slot/detail/def.hpp
hoxCommand.o: /usr/include/boost/function/detail/prologue.hpp
hoxCommand.o: /usr/include/boost/throw_exception.hpp
hoxCommand.o: /usr/include/boost/function/function_base.hpp
hoxCommand.o: /usr/include/boost/type_traits/composite_traits.hpp
hoxCommand.o: /usr/include/boost/ref.hpp
hoxCommand.o: /usr/include/boost/utility/addressof.hpp
hoxCommand.o: /usr/include/boost/type_traits/alignment_of.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/size_t_trait_def.hpp
hoxCommand.o: /usr/include/boost/mpl/size_t.hpp
hoxCommand.o: /usr/include/boost/mpl/size_t_fwd.hpp
hoxCommand.o: /usr/include/boost/type_traits/detail/size_t_trait_undef.hpp
hoxCommand.o: /usr/include/boost/utility/enable_if.hpp
hoxCommand.o: /usr/include/boost/function_equal.hpp
hoxCommand.o: /usr/include/boost/mem_fn.hpp
hoxCommand.o: /usr/include/boost/get_pointer.hpp
hoxCommand.o: /usr/include/boost/bind/mem_fn_template.hpp
hoxCommand.o: /usr/include/boost/bind/mem_fn_cc.hpp
hoxCommand.o: /usr/include/boost/preprocessor/enum.hpp
hoxCommand.o: /usr/include/boost/preprocessor/repetition/enum.hpp
hoxCommand.o: /usr/include/boost/preprocessor/tuple/rem.hpp
hoxCommand.o: /usr/include/boost/preprocessor/enum_params.hpp
hoxCommand.o: /usr/include/boost/preprocessor/repetition/enum_params.hpp
hoxCommand.o: /usr/include/boost/type_traits/function_traits.hpp
hoxCommand.o: /usr/include/boost/type_traits/is_base_and_derived.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/detail/case_conv.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/predicate.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/compare.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/find.hpp
hoxCommand.o: /usr/include/boost/range/iterator_range.hpp
hoxCommand.o: /usr/include/boost/range/functions.hpp
hoxCommand.o: /usr/include/boost/range/size.hpp
hoxCommand.o: /usr/include/boost/range/size_type.hpp
hoxCommand.o: /usr/include/boost/range/empty.hpp
hoxCommand.o: /usr/include/boost/range/rbegin.hpp
hoxCommand.o: /usr/include/boost/range/reverse_result_iterator.hpp
hoxCommand.o: /usr/include/boost/range/result_iterator.hpp
hoxCommand.o: /usr/include/boost/iterator/reverse_iterator.hpp
hoxCommand.o: /usr/include/boost/utility.hpp
hoxCommand.o: /usr/include/boost/utility/base_from_member.hpp
hoxCommand.o: /usr/include/boost/preprocessor/repetition/enum_binary_params.hpp
hoxCommand.o: /usr/include/boost/preprocessor/repetition/repeat_from_to.hpp
hoxCommand.o: /usr/include/boost/checked_delete.hpp
hoxCommand.o: /usr/include/boost/next_prior.hpp
hoxCommand.o: /usr/include/boost/noncopyable.hpp
hoxCommand.o: /usr/include/boost/range/reverse_iterator.hpp
hoxCommand.o: /usr/include/boost/range/const_reverse_iterator.hpp
hoxCommand.o: /usr/include/boost/range/rend.hpp
hoxCommand.o: /usr/include/boost/range/difference_type.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/finder.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/constants.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/detail/finder.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/detail/predicate.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/split.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/iter_find.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/concept.hpp
hoxCommand.o: /usr/include/boost/concept_check.hpp
hoxCommand.o: /usr/include/boost/type_traits/conversion_traits.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/find_iterator.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/detail/find_iterator.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/detail/util.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/join.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/detail/sequence.hpp
hoxCommand.o: /usr/include/boost/mpl/logical.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/replace.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/find_format.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/detail/find_format.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/detail/find_format_store.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/detail/replace_storage.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/detail/find_format_all.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/formatter.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/detail/formatter.hpp
hoxCommand.o: /usr/include/boost/algorithm/string/erase.hpp
AIPlayer.o: AIPlayer.h TcpLib.h hoxCommon.h /usr/include/boost/tokenizer.hpp
AIPlayer.o: /usr/include/boost/token_iterator.hpp
AIPlayer.o: /usr/include/boost/assert.hpp /usr/include/assert.h
AIPlayer.o: /usr/include/features.h /usr/include/sys/cdefs.h
AIPlayer.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
AIPlayer.o: /usr/include/gnu/stubs-64.h
AIPlayer.o: /usr/include/boost/iterator/iterator_adaptor.hpp
AIPlayer.o: /usr/include/boost/static_assert.hpp
AIPlayer.o: /usr/include/boost/config.hpp /usr/include/boost/config/user.hpp
AIPlayer.o: /usr/include/boost/config/select_compiler_config.hpp
AIPlayer.o: /usr/include/boost/config/compiler/gcc.hpp
AIPlayer.o: /usr/include/boost/config/select_stdlib_config.hpp
AIPlayer.o: /usr/include/boost/config/no_tr1/utility.hpp
AIPlayer.o: /usr/include/boost/config/select_platform_config.hpp
AIPlayer.o: /usr/include/boost/config/posix_features.hpp
AIPlayer.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
AIPlayer.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
AIPlayer.o: /usr/include/bits/confname.h /usr/include/getopt.h
AIPlayer.o: /usr/include/boost/config/suffix.hpp
AIPlayer.o: /usr/include/boost/detail/workaround.hpp
AIPlayer.o: /usr/include/boost/iterator.hpp /usr/include/boost/config.hpp
AIPlayer.o: /usr/include/boost/detail/iterator.hpp
AIPlayer.o: /usr/include/boost/iterator/iterator_categories.hpp
AIPlayer.o: /usr/include/boost/iterator/detail/config_def.hpp
AIPlayer.o: /usr/include/boost/mpl/eval_if.hpp /usr/include/boost/mpl/if.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/value_wknd.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/static_cast.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/workaround.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/integral.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/msvc.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/eti.hpp
AIPlayer.o: /usr/include/boost/mpl/int.hpp /usr/include/boost/mpl/int_fwd.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/adl_barrier.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/adl.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/intel.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/gcc.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/nttp_decl.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/nttp.hpp
AIPlayer.o: /usr/include/boost/preprocessor/cat.hpp
AIPlayer.o: /usr/include/boost/preprocessor/config/config.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/integral_wrapper.hpp
AIPlayer.o: /usr/include/boost/mpl/integral_c_tag.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/static_constant.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/na_spec.hpp
AIPlayer.o: /usr/include/boost/mpl/lambda_fwd.hpp
AIPlayer.o: /usr/include/boost/mpl/void_fwd.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/na.hpp
AIPlayer.o: /usr/include/boost/mpl/bool.hpp
AIPlayer.o: /usr/include/boost/mpl/bool_fwd.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/na_fwd.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/ctps.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/lambda.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/ttp.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/lambda_arity_param.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/template_arity_fwd.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/arity.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/dtp.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/preprocessor/params.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/preprocessor.hpp
AIPlayer.o: /usr/include/boost/preprocessor/comma_if.hpp
AIPlayer.o: /usr/include/boost/preprocessor/punctuation/comma_if.hpp
AIPlayer.o: /usr/include/boost/preprocessor/control/if.hpp
AIPlayer.o: /usr/include/boost/preprocessor/control/iif.hpp
AIPlayer.o: /usr/include/boost/preprocessor/logical/bool.hpp
AIPlayer.o: /usr/include/boost/preprocessor/facilities/empty.hpp
AIPlayer.o: /usr/include/boost/preprocessor/punctuation/comma.hpp
AIPlayer.o: /usr/include/boost/preprocessor/repeat.hpp
AIPlayer.o: /usr/include/boost/preprocessor/repetition/repeat.hpp
AIPlayer.o: /usr/include/boost/preprocessor/debug/error.hpp
AIPlayer.o: /usr/include/boost/preprocessor/detail/auto_rec.hpp
AIPlayer.o: /usr/include/boost/preprocessor/tuple/eat.hpp
AIPlayer.o: /usr/include/boost/preprocessor/inc.hpp
AIPlayer.o: /usr/include/boost/preprocessor/arithmetic/inc.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/preprocessor/enum.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/preprocessor/def_params_tail.hpp
AIPlayer.o: /usr/include/boost/mpl/limits/arity.hpp
AIPlayer.o: /usr/include/boost/preprocessor/logical/and.hpp
AIPlayer.o: /usr/include/boost/preprocessor/logical/bitand.hpp
AIPlayer.o: /usr/include/boost/preprocessor/identity.hpp
AIPlayer.o: /usr/include/boost/preprocessor/facilities/identity.hpp
AIPlayer.o: /usr/include/boost/preprocessor/empty.hpp
AIPlayer.o: /usr/include/boost/preprocessor/arithmetic/add.hpp
AIPlayer.o: /usr/include/boost/preprocessor/arithmetic/dec.hpp
AIPlayer.o: /usr/include/boost/preprocessor/control/while.hpp
AIPlayer.o: /usr/include/boost/preprocessor/list/fold_left.hpp
AIPlayer.o: /usr/include/boost/preprocessor/list/detail/fold_left.hpp
AIPlayer.o: /usr/include/boost/preprocessor/control/expr_iif.hpp
AIPlayer.o: /usr/include/boost/preprocessor/list/adt.hpp
AIPlayer.o: /usr/include/boost/preprocessor/detail/is_binary.hpp
AIPlayer.o: /usr/include/boost/preprocessor/detail/check.hpp
AIPlayer.o: /usr/include/boost/preprocessor/logical/compl.hpp
AIPlayer.o: /usr/include/boost/preprocessor/list/fold_right.hpp
AIPlayer.o: /usr/include/boost/preprocessor/list/detail/fold_right.hpp
AIPlayer.o: /usr/include/boost/preprocessor/list/reverse.hpp
AIPlayer.o: /usr/include/boost/preprocessor/control/detail/while.hpp
AIPlayer.o: /usr/include/boost/preprocessor/tuple/elem.hpp
AIPlayer.o: /usr/include/boost/preprocessor/arithmetic/sub.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/overload_resolution.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/lambda_support.hpp
AIPlayer.o: /usr/include/boost/mpl/identity.hpp
AIPlayer.o: /usr/include/boost/mpl/placeholders.hpp
AIPlayer.o: /usr/include/boost/mpl/arg.hpp /usr/include/boost/mpl/arg_fwd.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/na_assert.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/arity_spec.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/arg_typedef.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/use_preprocessed.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/compiler.hpp
AIPlayer.o: /usr/include/boost/preprocessor/stringize.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_convertible.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/yes_no_type.hpp
AIPlayer.o: /usr/include/boost/type_traits/config.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_array.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/bool_trait_def.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/template_arity_spec.hpp
AIPlayer.o: /usr/include/boost/type_traits/integral_constant.hpp
AIPlayer.o: /usr/include/boost/mpl/integral_c.hpp
AIPlayer.o: /usr/include/boost/mpl/integral_c_fwd.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/bool_trait_undef.hpp
AIPlayer.o: /usr/include/boost/type_traits/add_reference.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_reference.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/type_trait_def.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/type_trait_undef.hpp
AIPlayer.o: /usr/include/boost/type_traits/ice.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/ice_or.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/ice_and.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/ice_not.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/ice_eq.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_arithmetic.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_integral.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_float.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_void.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_abstract.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_class.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_union.hpp
AIPlayer.o: /usr/include/boost/type_traits/remove_cv.hpp
AIPlayer.o: /usr/include/boost/type_traits/broken_compiler_spec.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/cv_traits_impl.hpp
AIPlayer.o: /usr/include/boost/type_traits/intrinsics.hpp
AIPlayer.o: /usr/include/boost/iterator/detail/config_undef.hpp
AIPlayer.o: /usr/include/boost/iterator/iterator_facade.hpp
AIPlayer.o: /usr/include/boost/iterator/interoperable.hpp
AIPlayer.o: /usr/include/boost/mpl/or.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/nested_type_wknd.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
AIPlayer.o: /usr/include/boost/iterator/iterator_traits.hpp
AIPlayer.o: /usr/include/boost/iterator/detail/facade_iterator_category.hpp
AIPlayer.o: /usr/include/boost/mpl/and.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_same.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_const.hpp
AIPlayer.o: /usr/include/boost/detail/indirect_traits.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_function.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/false_result.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/is_function_ptr_helper.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_pointer.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_member_pointer.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_member_function_pointer.hpp
AIPlayer.o: /usr/include/boost/type_traits/detail/is_mem_fun_pointer_impl.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_volatile.hpp
AIPlayer.o: /usr/include/boost/type_traits/remove_reference.hpp
AIPlayer.o: /usr/include/boost/type_traits/remove_pointer.hpp
AIPlayer.o: /usr/include/boost/mpl/not.hpp
AIPlayer.o: /usr/include/boost/iterator/detail/enable_if.hpp
AIPlayer.o: /usr/include/boost/implicit_cast.hpp
AIPlayer.o: /usr/include/boost/type_traits/add_const.hpp
AIPlayer.o: /usr/include/boost/type_traits/add_pointer.hpp
AIPlayer.o: /usr/include/boost/type_traits/remove_const.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_pod.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_scalar.hpp
AIPlayer.o: /usr/include/boost/type_traits/is_enum.hpp
AIPlayer.o: /usr/include/boost/mpl/always.hpp
AIPlayer.o: /usr/include/boost/mpl/apply.hpp
AIPlayer.o: /usr/include/boost/mpl/apply_fwd.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
AIPlayer.o: /usr/include/boost/mpl/apply_wrap.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/has_apply.hpp
AIPlayer.o: /usr/include/boost/mpl/has_xxx.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/type_wrapper.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/yes_no.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/arrays.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/has_xxx.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/msvc_typename.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/has_apply.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/msvc_never_true.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
AIPlayer.o: /usr/include/boost/mpl/lambda.hpp /usr/include/boost/mpl/bind.hpp
AIPlayer.o: /usr/include/boost/mpl/bind_fwd.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/config/bind.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
AIPlayer.o: /usr/include/boost/mpl/next.hpp
AIPlayer.o: /usr/include/boost/mpl/next_prior.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/common_name_wknd.hpp
AIPlayer.o: /usr/include/boost/mpl/protect.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/full_lambda.hpp
AIPlayer.o: /usr/include/boost/mpl/quote.hpp /usr/include/boost/mpl/void.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/has_type.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/template_arity.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
AIPlayer.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
AIPlayer.o: /usr/include/boost/iterator/detail/minimum_category.hpp
AIPlayer.o: /usr/include/boost/token_functions.hpp ../folium/folHOXEngine.h
AIPlayer.o: hoxCommand.h
main.o: hoxCommon.h /usr/include/boost/tokenizer.hpp
main.o: /usr/include/boost/token_iterator.hpp /usr/include/boost/assert.hpp
main.o: /usr/include/assert.h /usr/include/features.h
main.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
main.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-64.h
main.o: /usr/include/boost/iterator/iterator_adaptor.hpp
main.o: /usr/include/boost/static_assert.hpp /usr/include/boost/config.hpp
main.o: /usr/include/boost/config/user.hpp
main.o: /usr/include/boost/config/select_compiler_config.hpp
main.o: /usr/include/boost/config/compiler/gcc.hpp
main.o: /usr/include/boost/config/select_stdlib_config.hpp
main.o: /usr/include/boost/config/no_tr1/utility.hpp
main.o: /usr/include/boost/config/select_platform_config.hpp
main.o: /usr/include/boost/config/posix_features.hpp /usr/include/unistd.h
main.o: /usr/include/bits/posix_opt.h /usr/include/bits/types.h
main.o: /usr/include/bits/typesizes.h /usr/include/bits/confname.h
main.o: /usr/include/getopt.h /usr/include/boost/config/suffix.hpp
main.o: /usr/include/boost/detail/workaround.hpp
main.o: /usr/include/boost/iterator.hpp /usr/include/boost/config.hpp
main.o: /usr/include/boost/detail/iterator.hpp
main.o: /usr/include/boost/iterator/iterator_categories.hpp
main.o: /usr/include/boost/iterator/detail/config_def.hpp
main.o: /usr/include/boost/mpl/eval_if.hpp /usr/include/boost/mpl/if.hpp
main.o: /usr/include/boost/mpl/aux_/value_wknd.hpp
main.o: /usr/include/boost/mpl/aux_/static_cast.hpp
main.o: /usr/include/boost/mpl/aux_/config/workaround.hpp
main.o: /usr/include/boost/mpl/aux_/config/integral.hpp
main.o: /usr/include/boost/mpl/aux_/config/msvc.hpp
main.o: /usr/include/boost/mpl/aux_/config/eti.hpp
main.o: /usr/include/boost/mpl/int.hpp /usr/include/boost/mpl/int_fwd.hpp
main.o: /usr/include/boost/mpl/aux_/adl_barrier.hpp
main.o: /usr/include/boost/mpl/aux_/config/adl.hpp
main.o: /usr/include/boost/mpl/aux_/config/intel.hpp
main.o: /usr/include/boost/mpl/aux_/config/gcc.hpp
main.o: /usr/include/boost/mpl/aux_/nttp_decl.hpp
main.o: /usr/include/boost/mpl/aux_/config/nttp.hpp
main.o: /usr/include/boost/preprocessor/cat.hpp
main.o: /usr/include/boost/preprocessor/config/config.hpp
main.o: /usr/include/boost/mpl/aux_/integral_wrapper.hpp
main.o: /usr/include/boost/mpl/integral_c_tag.hpp
main.o: /usr/include/boost/mpl/aux_/config/static_constant.hpp
main.o: /usr/include/boost/mpl/aux_/na_spec.hpp
main.o: /usr/include/boost/mpl/lambda_fwd.hpp
main.o: /usr/include/boost/mpl/void_fwd.hpp
main.o: /usr/include/boost/mpl/aux_/na.hpp /usr/include/boost/mpl/bool.hpp
main.o: /usr/include/boost/mpl/bool_fwd.hpp
main.o: /usr/include/boost/mpl/aux_/na_fwd.hpp
main.o: /usr/include/boost/mpl/aux_/config/ctps.hpp
main.o: /usr/include/boost/mpl/aux_/config/lambda.hpp
main.o: /usr/include/boost/mpl/aux_/config/ttp.hpp
main.o: /usr/include/boost/mpl/aux_/lambda_arity_param.hpp
main.o: /usr/include/boost/mpl/aux_/template_arity_fwd.hpp
main.o: /usr/include/boost/mpl/aux_/arity.hpp
main.o: /usr/include/boost/mpl/aux_/config/dtp.hpp
main.o: /usr/include/boost/mpl/aux_/preprocessor/params.hpp
main.o: /usr/include/boost/mpl/aux_/config/preprocessor.hpp
main.o: /usr/include/boost/preprocessor/comma_if.hpp
main.o: /usr/include/boost/preprocessor/punctuation/comma_if.hpp
main.o: /usr/include/boost/preprocessor/control/if.hpp
main.o: /usr/include/boost/preprocessor/control/iif.hpp
main.o: /usr/include/boost/preprocessor/logical/bool.hpp
main.o: /usr/include/boost/preprocessor/facilities/empty.hpp
main.o: /usr/include/boost/preprocessor/punctuation/comma.hpp
main.o: /usr/include/boost/preprocessor/repeat.hpp
main.o: /usr/include/boost/preprocessor/repetition/repeat.hpp
main.o: /usr/include/boost/preprocessor/debug/error.hpp
main.o: /usr/include/boost/preprocessor/detail/auto_rec.hpp
main.o: /usr/include/boost/preprocessor/tuple/eat.hpp
main.o: /usr/include/boost/preprocessor/inc.hpp
main.o: /usr/include/boost/preprocessor/arithmetic/inc.hpp
main.o: /usr/include/boost/mpl/aux_/preprocessor/enum.hpp
main.o: /usr/include/boost/mpl/aux_/preprocessor/def_params_tail.hpp
main.o: /usr/include/boost/mpl/limits/arity.hpp
main.o: /usr/include/boost/preprocessor/logical/and.hpp
main.o: /usr/include/boost/preprocessor/logical/bitand.hpp
main.o: /usr/include/boost/preprocessor/identity.hpp
main.o: /usr/include/boost/preprocessor/facilities/identity.hpp
main.o: /usr/include/boost/preprocessor/empty.hpp
main.o: /usr/include/boost/preprocessor/arithmetic/add.hpp
main.o: /usr/include/boost/preprocessor/arithmetic/dec.hpp
main.o: /usr/include/boost/preprocessor/control/while.hpp
main.o: /usr/include/boost/preprocessor/list/fold_left.hpp
main.o: /usr/include/boost/preprocessor/list/detail/fold_left.hpp
main.o: /usr/include/boost/preprocessor/control/expr_iif.hpp
main.o: /usr/include/boost/preprocessor/list/adt.hpp
main.o: /usr/include/boost/preprocessor/detail/is_binary.hpp
main.o: /usr/include/boost/preprocessor/detail/check.hpp
main.o: /usr/include/boost/preprocessor/logical/compl.hpp
main.o: /usr/include/boost/preprocessor/list/fold_right.hpp
main.o: /usr/include/boost/preprocessor/list/detail/fold_right.hpp
main.o: /usr/include/boost/preprocessor/list/reverse.hpp
main.o: /usr/include/boost/preprocessor/control/detail/while.hpp
main.o: /usr/include/boost/preprocessor/tuple/elem.hpp
main.o: /usr/include/boost/preprocessor/arithmetic/sub.hpp
main.o: /usr/include/boost/mpl/aux_/config/overload_resolution.hpp
main.o: /usr/include/boost/mpl/aux_/lambda_support.hpp
main.o: /usr/include/boost/mpl/identity.hpp
main.o: /usr/include/boost/mpl/placeholders.hpp
main.o: /usr/include/boost/mpl/arg.hpp /usr/include/boost/mpl/arg_fwd.hpp
main.o: /usr/include/boost/mpl/aux_/na_assert.hpp
main.o: /usr/include/boost/mpl/aux_/arity_spec.hpp
main.o: /usr/include/boost/mpl/aux_/arg_typedef.hpp
main.o: /usr/include/boost/mpl/aux_/config/use_preprocessed.hpp
main.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
main.o: /usr/include/boost/mpl/aux_/config/compiler.hpp
main.o: /usr/include/boost/preprocessor/stringize.hpp
main.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
main.o: /usr/include/boost/type_traits/is_convertible.hpp
main.o: /usr/include/boost/type_traits/detail/yes_no_type.hpp
main.o: /usr/include/boost/type_traits/config.hpp
main.o: /usr/include/boost/type_traits/is_array.hpp
main.o: /usr/include/boost/type_traits/detail/bool_trait_def.hpp
main.o: /usr/include/boost/type_traits/detail/template_arity_spec.hpp
main.o: /usr/include/boost/type_traits/integral_constant.hpp
main.o: /usr/include/boost/mpl/integral_c.hpp
main.o: /usr/include/boost/mpl/integral_c_fwd.hpp
main.o: /usr/include/boost/type_traits/detail/bool_trait_undef.hpp
main.o: /usr/include/boost/type_traits/add_reference.hpp
main.o: /usr/include/boost/type_traits/is_reference.hpp
main.o: /usr/include/boost/type_traits/detail/type_trait_def.hpp
main.o: /usr/include/boost/type_traits/detail/type_trait_undef.hpp
main.o: /usr/include/boost/type_traits/ice.hpp
main.o: /usr/include/boost/type_traits/detail/ice_or.hpp
main.o: /usr/include/boost/type_traits/detail/ice_and.hpp
main.o: /usr/include/boost/type_traits/detail/ice_not.hpp
main.o: /usr/include/boost/type_traits/detail/ice_eq.hpp
main.o: /usr/include/boost/type_traits/is_arithmetic.hpp
main.o: /usr/include/boost/type_traits/is_integral.hpp
main.o: /usr/include/boost/type_traits/is_float.hpp
main.o: /usr/include/boost/type_traits/is_void.hpp
main.o: /usr/include/boost/type_traits/is_abstract.hpp
main.o: /usr/include/boost/type_traits/is_class.hpp
main.o: /usr/include/boost/type_traits/is_union.hpp
main.o: /usr/include/boost/type_traits/remove_cv.hpp
main.o: /usr/include/boost/type_traits/broken_compiler_spec.hpp
main.o: /usr/include/boost/type_traits/detail/cv_traits_impl.hpp
main.o: /usr/include/boost/type_traits/intrinsics.hpp
main.o: /usr/include/boost/iterator/detail/config_undef.hpp
main.o: /usr/include/boost/iterator/iterator_facade.hpp
main.o: /usr/include/boost/iterator/interoperable.hpp
main.o: /usr/include/boost/mpl/or.hpp
main.o: /usr/include/boost/mpl/aux_/nested_type_wknd.hpp
main.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
main.o: /usr/include/boost/iterator/iterator_traits.hpp
main.o: /usr/include/boost/iterator/detail/facade_iterator_category.hpp
main.o: /usr/include/boost/mpl/and.hpp
main.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
main.o: /usr/include/boost/type_traits/is_same.hpp
main.o: /usr/include/boost/type_traits/is_const.hpp
main.o: /usr/include/boost/detail/indirect_traits.hpp
main.o: /usr/include/boost/type_traits/is_function.hpp
main.o: /usr/include/boost/type_traits/detail/false_result.hpp
main.o: /usr/include/boost/type_traits/detail/is_function_ptr_helper.hpp
main.o: /usr/include/boost/type_traits/is_pointer.hpp
main.o: /usr/include/boost/type_traits/is_member_pointer.hpp
main.o: /usr/include/boost/type_traits/is_member_function_pointer.hpp
main.o: /usr/include/boost/type_traits/detail/is_mem_fun_pointer_impl.hpp
main.o: /usr/include/boost/type_traits/is_volatile.hpp
main.o: /usr/include/boost/type_traits/remove_reference.hpp
main.o: /usr/include/boost/type_traits/remove_pointer.hpp
main.o: /usr/include/boost/mpl/not.hpp
main.o: /usr/include/boost/iterator/detail/enable_if.hpp
main.o: /usr/include/boost/implicit_cast.hpp
main.o: /usr/include/boost/type_traits/add_const.hpp
main.o: /usr/include/boost/type_traits/add_pointer.hpp
main.o: /usr/include/boost/type_traits/remove_const.hpp
main.o: /usr/include/boost/type_traits/is_pod.hpp
main.o: /usr/include/boost/type_traits/is_scalar.hpp
main.o: /usr/include/boost/type_traits/is_enum.hpp
main.o: /usr/include/boost/mpl/always.hpp /usr/include/boost/mpl/apply.hpp
main.o: /usr/include/boost/mpl/apply_fwd.hpp
main.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
main.o: /usr/include/boost/mpl/apply_wrap.hpp
main.o: /usr/include/boost/mpl/aux_/has_apply.hpp
main.o: /usr/include/boost/mpl/has_xxx.hpp
main.o: /usr/include/boost/mpl/aux_/type_wrapper.hpp
main.o: /usr/include/boost/mpl/aux_/yes_no.hpp
main.o: /usr/include/boost/mpl/aux_/config/arrays.hpp
main.o: /usr/include/boost/mpl/aux_/config/has_xxx.hpp
main.o: /usr/include/boost/mpl/aux_/config/msvc_typename.hpp
main.o: /usr/include/boost/mpl/aux_/config/has_apply.hpp
main.o: /usr/include/boost/mpl/aux_/msvc_never_true.hpp
main.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
main.o: /usr/include/boost/mpl/lambda.hpp /usr/include/boost/mpl/bind.hpp
main.o: /usr/include/boost/mpl/bind_fwd.hpp
main.o: /usr/include/boost/mpl/aux_/config/bind.hpp
main.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
main.o: /usr/include/boost/mpl/next.hpp /usr/include/boost/mpl/next_prior.hpp
main.o: /usr/include/boost/mpl/aux_/common_name_wknd.hpp
main.o: /usr/include/boost/mpl/protect.hpp
main.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
main.o: /usr/include/boost/mpl/aux_/full_lambda.hpp
main.o: /usr/include/boost/mpl/quote.hpp /usr/include/boost/mpl/void.hpp
main.o: /usr/include/boost/mpl/aux_/has_type.hpp
main.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
main.o: /usr/include/boost/mpl/aux_/template_arity.hpp
main.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
main.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
main.o: /usr/include/boost/mpl/aux_/include_preprocessed.hpp
main.o: /usr/include/boost/iterator/detail/minimum_category.hpp
main.o: /usr/include/boost/token_functions.hpp TcpLib.h AIPlayer.h
main.o: ../folium/folHOXEngine.h hoxCommand.h
