/*
Copyright (C) 2001-present by Serge Lamikhov-Center

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifdef _MSC_VER
#define _SCL_SECURE_NO_WARNINGS
#define ELFIO_NO_INTTYPES
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <elfio/elfio.hpp>

using namespace ELFIO;

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( modinfo_read )
{
    elfio reader;
    BOOST_REQUIRE_EQUAL( reader.load( "elf_examples/zavl.ko" ), true );

    section* modinfo_sec = reader.sections[".modinfo"];
    BOOST_REQUIRE_NE( modinfo_sec, nullptr );

    const_modinfo_section_accessor modinfo( modinfo_sec );
    BOOST_REQUIRE_EQUAL( modinfo.get_attribute_num(), (Elf_Word)9 );

    struct
    {
        std::string field;
        std::string value;
    } attributes[] = { { "version", "0.8.3-1ubuntu12.1" },
                       { "license", "CDDL" },
                       { "author", "OpenZFS on Linux" },
                       { "description", "Generic AVL tree implementation" },
                       { "srcversion", "98E85778E754CF75DEF9E8E" },
                       { "depends", "spl" },
                       { "retpoline", "Y" },
                       { "name", "zavl" },
                       { "vermagic", "5.4.0-42-generic SMP mod_unload " } };

    for ( auto i = 0; i < sizeof( attributes ) / sizeof( attributes[0] );
          i++ ) {
        std::string field;
        std::string value;
        modinfo.get_attribute( i, field, value );

        BOOST_CHECK_EQUAL( field, attributes[i].field );
        BOOST_CHECK_EQUAL( value, attributes[i].value );
    }

    for ( auto i = 0; i < sizeof( attributes ) / sizeof( attributes[0] );
          i++ ) {
        std::string field = attributes[i].field;
        std::string value;
        modinfo.get_attribute( field, value );

        BOOST_CHECK_EQUAL( value, attributes[i].value );
    }
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( modinfo_write )
{
    elfio writer;
    BOOST_REQUIRE_EQUAL( writer.load( "elf_examples/zavl.ko" ), true );

    section* modinfo_sec = writer.sections[".modinfo"];
    BOOST_REQUIRE_NE( modinfo_sec, nullptr );

    modinfo_section_accessor modinfo( modinfo_sec );
    BOOST_REQUIRE_EQUAL( modinfo.get_attribute_num(), (Elf_Word)9 );

    modinfo.add_attribute( "test1", "value1" );
    modinfo.add_attribute( "test2", "value2" );

    BOOST_REQUIRE_EQUAL( modinfo.get_attribute_num(), (Elf_Word)11 );

    BOOST_REQUIRE_EQUAL( writer.save( "elf_examples/zavl_gen.ko" ), true );

    elfio reader;
    BOOST_REQUIRE_EQUAL( reader.load( "elf_examples/zavl_gen.ko" ), true );

    modinfo_sec = reader.sections[".modinfo"];
    BOOST_REQUIRE_NE( modinfo_sec, nullptr );

    const_modinfo_section_accessor modinfo1( modinfo_sec );
    BOOST_REQUIRE_EQUAL( modinfo1.get_attribute_num(), (Elf_Word)11 );

    struct
    {
        std::string field;
        std::string value;
    } attributes[] = { { "version", "0.8.3-1ubuntu12.1" },
                       { "license", "CDDL" },
                       { "author", "OpenZFS on Linux" },
                       { "description", "Generic AVL tree implementation" },
                       { "srcversion", "98E85778E754CF75DEF9E8E" },
                       { "depends", "spl" },
                       { "retpoline", "Y" },
                       { "name", "zavl" },
                       { "vermagic", "5.4.0-42-generic SMP mod_unload " },
                       { "test1", "value1" },
                       { "test2", "value2" } };

    for ( auto i = 0; i < sizeof( attributes ) / sizeof( attributes[0] );
          i++ ) {
        std::string field;
        std::string value;
        modinfo.get_attribute( i, field, value );

        BOOST_CHECK_EQUAL( field, attributes[i].field );
        BOOST_CHECK_EQUAL( value, attributes[i].value );
    }

    for ( auto i = 0; i < sizeof( attributes ) / sizeof( attributes[0] );
          i++ ) {
        std::string field = attributes[i].field;
        std::string value;
        modinfo.get_attribute( field, value );

        BOOST_CHECK_EQUAL( value, attributes[i].value );
    }
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( array_read_32 )
{
    elfio reader;
    BOOST_REQUIRE_EQUAL( reader.load( "elf_examples/hello_32" ), true );

    section* array_sec = reader.sections[".ctors"];
    BOOST_REQUIRE_NE( array_sec, nullptr );

    const_array_section_accessor<> array( reader, array_sec );
    BOOST_REQUIRE_EQUAL( array.get_entries_num(), (Elf_Xword)2 );
    Elf64_Addr addr;
    BOOST_CHECK_EQUAL( array.get_entry( 0, addr ), true );
    BOOST_CHECK_EQUAL( addr, 0xFFFFFFFF );
    BOOST_CHECK_EQUAL( array.get_entry( 1, addr ), true );
    BOOST_CHECK_EQUAL( addr, 0x00000000 );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( array_read_64 )
{
    elfio reader;
    BOOST_REQUIRE_EQUAL( reader.load( "elf_examples/hello_64" ), true );

    section* array_sec = reader.sections[".ctors"];
    BOOST_REQUIRE_NE( array_sec, nullptr );

    const_array_section_accessor<Elf64_Addr> array( reader, array_sec );
    BOOST_REQUIRE_EQUAL( array.get_entries_num(), (Elf_Xword)2 );
    Elf64_Addr addr;
    BOOST_CHECK_EQUAL( array.get_entry( 0, addr ), true );
    BOOST_CHECK_EQUAL( addr, 0xFFFFFFFFFFFFFFFF );
    BOOST_CHECK_EQUAL( array.get_entry( 1, addr ), true );
    BOOST_CHECK_EQUAL( addr, 0x0000000000000000 );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( init_array_read_64 )
{
    elfio      reader;
    Elf64_Addr addr;
    BOOST_REQUIRE_EQUAL( reader.load( "elf_examples/ctors" ), true );

    section* array_sec = reader.sections[".init_array"];
    BOOST_REQUIRE_NE( array_sec, nullptr );

    const_array_section_accessor<Elf64_Addr> array( reader, array_sec );
    BOOST_REQUIRE_EQUAL( array.get_entries_num(), (Elf_Xword)2 );
    BOOST_CHECK_EQUAL( array.get_entry( 0, addr ), true );
    BOOST_CHECK_EQUAL( addr, 0x12C0 );
    BOOST_CHECK_EQUAL( array.get_entry( 1, addr ), true );
    BOOST_CHECK_EQUAL( addr, 0x149F );

    array_sec = reader.sections[".fini_array"];
    BOOST_REQUIRE_NE( array_sec, nullptr );

    array_section_accessor<Elf64_Addr> arrayf( reader, array_sec );
    BOOST_REQUIRE_EQUAL( arrayf.get_entries_num(), (Elf_Xword)1 );
    BOOST_CHECK_EQUAL( arrayf.get_entry( 0, addr ), true );
    BOOST_CHECK_EQUAL( addr, 0x1280 );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( init_array_write_64 )
{
    elfio      reader;
    Elf64_Addr addr;
    BOOST_REQUIRE_EQUAL( reader.load( "elf_examples/ctors" ), true );

    section* array_sec = reader.sections[".init_array"];
    BOOST_REQUIRE_NE( array_sec, nullptr );

    array_section_accessor<Elf64_Addr> array( reader, array_sec );
    BOOST_REQUIRE_EQUAL( array.get_entries_num(), (Elf_Xword)2 );
    BOOST_CHECK_EQUAL( array.get_entry( 0, addr ), true );
    BOOST_CHECK_EQUAL( addr, 0x12C0 );
    BOOST_CHECK_EQUAL( array.get_entry( 1, addr ), true );
    BOOST_CHECK_EQUAL( addr, 0x149F );

    array.add_entry( 0x12345678 );

    BOOST_REQUIRE_EQUAL( array.get_entries_num(), (Elf_Xword)3 );
    BOOST_CHECK_EQUAL( array.get_entry( 0, addr ), true );
    BOOST_CHECK_EQUAL( addr, 0x12C0 );
    BOOST_CHECK_EQUAL( array.get_entry( 1, addr ), true );
    BOOST_CHECK_EQUAL( addr, 0x149F );
    BOOST_CHECK_EQUAL( array.get_entry( 2, addr ), true );
    BOOST_CHECK_EQUAL( addr, 0x12345678 );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( test_hex )
{
    BOOST_CHECK_EQUAL( to_hex_string( 1 ), "0x1" );
    BOOST_CHECK_EQUAL( to_hex_string( 10 ), "0xA" );
    BOOST_CHECK_EQUAL( to_hex_string( 0x12345678 ), "0x12345678" );
    BOOST_CHECK_EQUAL( to_hex_string( 0xFFFFFFFF ), "0xFFFFFFFF" );
    BOOST_CHECK_EQUAL( to_hex_string( 0xFFFFFFFFFFFFFFFF ),
                       "0xFFFFFFFFFFFFFFFF" );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( hash32_le )
{
    elfio reader;
    // Load ELF data

    BOOST_REQUIRE_EQUAL( reader.load( "elf_examples/ARMSCII-8.so" ), true );

    std::string             name;
    Elf64_Addr              value;
    Elf_Xword               size;
    unsigned char           bind;
    unsigned char           type;
    Elf_Half                section_index;
    unsigned char           other;
    section*                symsec = reader.sections[".dynsym"];
    symbol_section_accessor syms( reader, symsec );

    for ( int i = 0; i < syms.get_symbols_num(); i++ ) {
        BOOST_REQUIRE_EQUAL( syms.get_symbol( i, name, value, size, bind, type,
                                              section_index, other ),
                             true );
        BOOST_CHECK_EQUAL( syms.get_symbol( name, value, size, bind, type,
                                            section_index, other ),
                           true );
    }
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( hash32_be )
{
    elfio reader;
    // Load ELF data

    BOOST_REQUIRE_EQUAL( reader.load( "elf_examples/test_ppc" ), true );

    std::string             name;
    Elf64_Addr              value;
    Elf_Xword               size;
    unsigned char           bind;
    unsigned char           type;
    Elf_Half                section_index;
    unsigned char           other;
    section*                symsec = reader.sections[".dynsym"];
    symbol_section_accessor syms( reader, symsec );

    for ( int i = 0; i < syms.get_symbols_num(); i++ ) {
        BOOST_REQUIRE_EQUAL( syms.get_symbol( i, name, value, size, bind, type,
                                              section_index, other ),
                             true );
        BOOST_CHECK_EQUAL( syms.get_symbol( name, value, size, bind, type,
                                            section_index, other ),
                           true );
    }
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( gnu_hash32_le )
{
    elfio reader;
    // Load ELF data

    BOOST_REQUIRE_EQUAL( reader.load( "elf_examples/hello_32" ), true );

    std::string             name;
    Elf64_Addr              value;
    Elf_Xword               size;
    unsigned char           bind;
    unsigned char           type;
    Elf_Half                section_index;
    unsigned char           other;
    section*                symsec = reader.sections[".dynsym"];
    symbol_section_accessor syms( reader, symsec );

    for ( int i = 0; i < syms.get_symbols_num(); i++ ) {
        BOOST_REQUIRE_EQUAL( syms.get_symbol( i, name, value, size, bind, type,
                                              section_index, other ),
                             true );
        BOOST_CHECK_EQUAL( syms.get_symbol( name, value, size, bind, type,
                                            section_index, other ),
                           true );
    }
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( gnu_hash64_le )
{
    elfio reader;
    // Load ELF data

    BOOST_REQUIRE_EQUAL( reader.load( "elf_examples/main" ), true );

    std::string             name;
    Elf64_Addr              value;
    Elf_Xword               size;
    unsigned char           bind;
    unsigned char           type;
    Elf_Half                section_index;
    unsigned char           other;
    section*                symsec = reader.sections[".dynsym"];
    symbol_section_accessor syms( reader, symsec );

    for ( int i = 0; i < syms.get_symbols_num(); i++ ) {
        BOOST_REQUIRE_EQUAL( syms.get_symbol( i, name, value, size, bind, type,
                                              section_index, other ),
                             true );
        BOOST_CHECK_EQUAL( syms.get_symbol( name, value, size, bind, type,
                                            section_index, other ),
                           true );
    }
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( gnu_version_64_le )
{
    elfio reader;
    // Load ELF data

    BOOST_REQUIRE_EQUAL( reader.load( "elf_examples/hello_64" ), true );

    std::string                   name;
    Elf64_Addr                    value;
    Elf_Xword                     size;
    unsigned char                 bind;
    unsigned char                 type;
    Elf_Half                      section_index;
    unsigned char                 other;
    section*                      symsec = reader.sections[".dynsym"];
    const_symbol_section_accessor dynsym( reader, symsec );

    section* versec = reader.sections[".gnu.version"];
    const_array_section_accessor<Elf64_Half> vers( reader, versec );

    section* verneed = reader.sections[".gnu.version_r"];
    const_array_section_accessor<Elf64_Half> vern( reader, verneed );

    section* dynstr = reader.sections[".dynstr"];

    BOOST_CHECK_EQUAL( versec->get_link(), symsec->get_index() );
    BOOST_CHECK_EQUAL( verneed->get_link(), dynstr->get_index() );

    for ( int i = 0; i < dynsym.get_symbols_num(); i++ ) {
        BOOST_REQUIRE_EQUAL( dynsym.get_symbol( i, name, value, size, bind,
                                                type, section_index, other ),
                             true );

        Elf64_Addr verindex = 0;

        vers.get_entry( i, verindex );
        BOOST_CHECK_EQUAL( verindex, ( i / 2 ) * 2 );

        const char* ptr = verneed->get_data();
        if ( verindex > 1 ) {
            Elf64_Addr value = 0;
            vern.get_entry( verindex * 8, value );
            // std::cout << value << std::endl;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( move_constructor_and_assignment )
{
    elfio r1;

    // Load ELF data
    BOOST_REQUIRE_EQUAL( r1.load( "elf_examples/hello_64" ), true );
    Elf64_Addr  entry    = r1.get_entry();
    std::string sec_name = r1.sections[".text"]->get_name();
    Elf_Xword   seg_size = r1.segments[1]->get_memory_size();

    // Move to a vector element
    std::vector<elfio> v;
    v.emplace_back( std::move( r1 ) );
    BOOST_CHECK_EQUAL( v[0].get_entry(), entry );
    BOOST_CHECK_EQUAL( v[0].sections[".text"]->get_name(), sec_name );
    BOOST_CHECK_EQUAL( v[0].segments[1]->get_memory_size(), seg_size );

    elfio r2;
    r2 = std::move( v[0] );
    BOOST_CHECK_EQUAL( r2.get_entry(), entry );
    BOOST_CHECK_EQUAL( r2.sections[".text"]->get_name(), sec_name );
    BOOST_CHECK_EQUAL( r2.segments[1]->get_memory_size(), seg_size );
}
