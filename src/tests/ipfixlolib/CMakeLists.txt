ADD_EXECUTABLE(test_everything
	test_everything.cc
)

ADD_EXECUTABLE(mtutest
	mtutest.c
)

TARGET_LINK_LIBRARIES(mtutest
	ipfixlolib
	common
)
SET_TARGET_PROPERTIES(mtutest
	PROPERTIES
	LINKER_LANGUAGE CXX)

TARGET_LINK_LIBRARIES(test_everything
	ipfixlolib
	common
)

ADD_EXECUTABLE(example_code
	example_code.cc
)

TARGET_LINK_LIBRARIES(example_code
	ipfixlolib
	common
)

ADD_EXECUTABLE(example_code_2
	example_code_2.c
)

TARGET_LINK_LIBRARIES(example_code_2
	ipfixlolib
	common
)

SET_TARGET_PROPERTIES(example_code_2
	PROPERTIES
	LINKER_LANGUAGE CXX)

IF (SUPPORT_DTLS)
	TARGET_LINK_LIBRARIES(test_everything ${OPENSSL_LIBRARIES})
	TARGET_LINK_LIBRARIES(mtutest ${OPENSSL_LIBRARIES})
	TARGET_LINK_LIBRARIES(example_code ${OPENSSL_LIBRARIES})
	TARGET_LINK_LIBRARIES(example_code_2 ${OPENSSL_LIBRARIES})
ENDIF (SUPPORT_DTLS)
