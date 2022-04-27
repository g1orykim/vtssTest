#!/usr/bin/env ruby

require 'xml'
require 'active_support/core_ext/string/inflections.rb'

def cputs indent, str
    indent.times {|i| print "  " }
    puts str
end

def parse_reg_addres cnt, width, addr
    cnt = Integer(cnt)
    width = Integer(width)

    if cnt == 1
        return [Integer(addr)]
    else
        a = addr.split(' ')
        res = []
        if (a.size == 1)
            addr = Integer(addr)
            cnt.times do |i|
                res << addr + i * width
            end
            return res
        else
            a.each do |i|
                res << Integer(i)
            end
            return res
        end
    end
end

def print_reg params
    print "#{params[:target_channel]},#{params[:target_device]}@"
    print ("0x%04x " % params[:reg_addr])
    print "#{params[:target_name]}:#{params[:group_name]}"

    if params[:group_rep]
        print "[#{params[:group_idx]}]"
    end

    print ":#{params[:reg_name]}"

    if params[:reg_rep]
        print "[#{params[:reg_idx]}]"
    end

    print " #{params[:reg_defaults]}"

    print "\n"

    #if params[:group_rep]
    #    puts "0x%04x #{params[:target_name]}:#{params[:group_name]}[#{params[:group_idx]}]:#{@name}" % [a]
    #else
    #    puts "0x%04x #{params[:target_name]}:#{params[:group_name]}:#{@name}" % [a]
    #end
end

def mask_get width, pos
    val = 0
    width.times do |x|
        val = val << 1
        val = val | 0x1
    end
    val = val << pos

    return val
end

class Reg
    attr_accessor :name
    attr_accessor :addrs
    attr_accessor :class_name
    attr_accessor :class_instance

    def initialize name, addr, rep_cnt, rep_width
        @name = name
        @addrs = parse_reg_addres(rep_cnt, rep_width, addr)

        @max_width = 0
        @sw_read_mask  = 0
        @sw_write_mask = 0
        @hw_read_mask  = 0
        @hw_write_mask = 0
        @defaults = 0

        @class_name = "#{@name}_reg".downcase.camelize
        if (@addrs.size > 1)
            @class_instance = @name.downcase.camelize(:lower).pluralize
        else
            @class_instance = @name.downcase.camelize(:lower)
        end
    end

    def print_short
        puts "    #{@name} (#{@addrs.join(", ")})"
    end

    def update_mask mask, sw_read, sw_write, hw_read, hw_write
        if sw_read
            @sw_read_mask = @sw_read_mask | mask
        end
        if sw_write
            @sw_write_mask = @sw_write_mask | mask
        end
        if hw_read
            @hw_read_mask = @hw_read_mask | mask
        end
        if hw_write
            @hw_write_mask = @hw_write_mask | mask
        end
    end

    def parse doc
        # <field name="PMA_DEVICE_ID_1" type="ro" default="0x0007" width="16" pos="0">

        doc.find('./field').each do |field|
            mask = mask_get Integer(field['width']), Integer(field['pos'])

            w = Integer(field['width']) + Integer(field['pos'])
            if w > @max_width
                @max_width = w
            end

            case field["type"]
            when "ro"
                update_mask mask, true, false, true, true
            when "rw"
                update_mask mask, true, true, true, false
            when "cnt"
                update_mask mask, true, true, true, true
            when "he"
                update_mask mask, true, true, true, true
            when "se"
                update_mask mask, true, true, true, true
            when "rhw"
                update_mask mask, true, true, true, true
            when "ha_se"
                update_mask mask, true, true, true, true
            when "ha_cnt"
                update_mask mask, true, true, true, true
            when "ha_he"
                update_mask mask, true, true, true, true
            when "ha_rw"
                update_mask mask, true, true, true, false
            when "ha_ro"
                update_mask mask, true, false, true, true
            else
                raise "Unknown: #{field["type"]}"
            end

            begin
                val = Integer(field['default'])
                val = val << Integer(field['pos'])
                @defaults = @defaults | val
            rescue
            end
        end
    end

    def reglist params
        if @addrs.size > 1
            params[:reg_rep] = true
        else
            params[:reg_rep] = false
        end
        params[:reg_name] = @name
        params[:reg_sw_read_mask]  = @sw_read_mask 
        params[:reg_sw_write_mask] = @sw_write_mask
        params[:reg_hw_read_mask]  = @hw_read_mask 
        params[:reg_hw_write_mask] = @hw_write_mask
        params[:reg_defaults]      = @defaults

        @addrs.each_index do |idx|
            params[:reg_addr] = @addrs[idx] + params[:group_addr]
            params[:reg_idx] = idx
            print_reg(params)
        end
    end

    def hxx_common indent
        cputs indent + 1, "static const char *const NAME; // #{@name}"
        cputs indent + 1, "const char * name() const {"
        cputs indent + 2, "return NAME;"
        cputs indent + 1, "}"
        cputs indent + 1, "static const uint32_t INITIAL; // #{"0x%x"%[@defaults]}"
        cputs indent + 1, "const uint32_t initial() const {"
        cputs indent + 2, "return INITIAL;"
        cputs indent + 1, "}"
        cputs indent + 1, "static const uint32_t READ_MASK; // #{"0x%x"%[@sw_read_mask]}"
        cputs indent + 1, "const uint32_t read_mask() const {"
        cputs indent + 2, "return READ_MASK;"
        cputs indent + 1, "}"
        cputs indent + 1, "static const uint32_t MAX_WIDTH; // #{"0x%x"%[@max_width]}"
        cputs indent + 1, "const uint32_t max_width() const {"
        cputs indent + 2, "return MAX_WIDTH;"
        cputs indent + 1, "}"
        cputs indent + 1, "static const uint32_t WRITE_MASK; // #{"0x%x"%[@sw_write_mask]}"
        cputs indent + 1, "const uint32_t write_mask() const {"
        cputs indent + 2, "return WRITE_MASK;"
        cputs indent + 1, "}"
    end

    def hxx_single indent
        cputs indent, "struct #{@class_name} : public Register, public RegisterDesc"
        cputs indent, "{"
        hxx_common indent
        cputs indent + 1, "static const uint32_t ADDRESS; // #{"0x%x"%[@addrs[0]]}"
        cputs indent + 1, "void walk(Walker *w, Chip *c, Target *t, RegisterGroup *g);"
        cputs indent + 1, "void walk(Walker *w, Chip *c, Target *t, RegisterGroupSet *g, uint32_t idx);"
        cputs indent + 1, "uint32_t address() const {"
        cputs indent + 2, "return ADDRESS;"
        cputs indent + 1, "}"
        cputs indent, "};"
        cputs indent, "#{@class_name} #{@class_instance};"
    end

    def hxx_array indent
        cputs indent, "struct #{@class_name} : public RegisterSetDesc"
        cputs indent, "{"
        hxx_common indent
        cputs indent + 1, "static const uint32_t ADDRESSES[#{@addrs.size}]; //{#{@addrs.map{|x| "0x%x" % [x]}.join(", ")}}"
        cputs indent + 1, "uint32_t address(uint32_t idx) const {"
        cputs indent + 2, "return ADDRESSES[idx];"
        cputs indent + 1, "}"
        cputs indent + 1, "uint32_t cnt() const {"
        cputs indent + 2, "return #{@addrs.size};"
        cputs indent + 1, "}"
        cputs indent + 1, "Register reg[#{@addrs.size}];"
        cputs indent + 1, "Register& operator[](size_t s) {"
        cputs indent + 2, "return reg[s];"
        cputs indent + 1, "}"
        cputs indent + 1, "void walk(Walker *w, Chip *c, Target *t, RegisterGroup *g);"
        cputs indent + 1, "void walk(Walker *w, Chip *c, Target *t, RegisterGroupSet *g, uint32_t idx);"
        cputs indent, "};"
        cputs indent, "#{@class_name} #{@class_instance};"
    end

    def hxx indent
        if (@addrs.size > 1)
            hxx_array indent
        else
            hxx_single indent
        end
    end

    def cxx_single ns
        puts "const uint32_t #{ns}::ADDRESS = #{"0x%x"%[@addrs[0]]};"
    end

    def cxx_array ns
        puts "const uint32_t #{ns}::ADDRESSES[] = {#{@addrs.map{|x| "0x%x" % [x]}.join(", ")}};"
    end

    def cxx ns
        ns = "#{ns}::#{@class_name}"
        puts "const char *const #{ns}::NAME = \"#{@name}\";"
        puts "const uint32_t #{ns}::INITIAL = #{"0x%x"%[@defaults]};"
        puts "const uint32_t #{ns}::READ_MASK = #{"0x%x"%[@sw_read_mask]};"
        puts "const uint32_t #{ns}::MAX_WIDTH= #{"0x%x"%[@max_width]};"
        puts "const uint32_t #{ns}::WRITE_MASK = #{"0x%x"%[@sw_write_mask]};"
        cputs 0, "void #{ns}::walk(Walker *w, Chip *c, Target *t, RegisterGroup *g)"
        cputs 0, "{"
        if (@addrs.size > 1)
            cputs 1, "for (int i = 0; i < #{@addrs.size}; ++i)"
            cputs 2, "w->exec(c, t, g, this, i, &reg[i]);"
        else
            cputs 1, "w->exec(c, t, g, this, this);"
        end
        cputs 0, "}"
        cputs 0, "void #{ns}::walk(Walker *w, Chip *c, Target *t, RegisterGroupSet *g, uint32_t idx)"
        cputs 0, "{"
        if (@addrs.size > 1)
            cputs 1, "for (int i = 0; i < #{@addrs.size}; ++i)"
            cputs 2, "w->exec(c, t, g, idx, this, i, &reg[i]);"
        else
            cputs 1, "w->exec(c, t, g, idx, this, this);"
        end
        cputs 0, "}"

        if (@addrs.size > 1)
            cxx_array ns
        else
            cxx_single ns
        end
    end
end

class RegGrp
    attr_accessor :name
    attr_accessor :base_addr
    attr_accessor :regs
    attr_accessor :class_name
    attr_accessor :class_instance

    def initialize name, base, rep_cnt, rep_width
        @name = name
        @addrs = parse_reg_addres(rep_cnt, rep_width, base)
        @regs = []

        @class_base_name = @name.downcase
        @class_name_cnt = "#{@name.downcase}_cnt".camelize
        @class_name = @class_base_name.camelize

        if (@addrs.size > 1)
            @class_instance = @class_base_name.camelize(:lower).pluralize
        else
            @class_instance = @class_base_name.camelize(:lower)
        end
    end

    def parse doc
        doc.find('.//reg').each do |reg|
            r = Reg.new reg['name'], reg['addr'], reg['repl_cnt'], reg['repl_width']
            r.parse reg
            @regs << r
        end
    end

    def print_short
        puts "  #{@name} (#{@addrs.join(", ")})"
        @regs.each do |r|
            r.print_short
        end
    end

    def reglist params
        if @addrs.size > 1
            params[:group_rep] = true
        else
            params[:group_rep] = false
        end

        params[:group_name] = @name


        @addrs.each_index do |idx|
            params[:group_idx] = idx
            params[:group_addr] = @addrs[idx]
            @regs.each do |reg|
                reg.reglist params
            end
        end
    end

    def hxx_single indent
        cputs indent, "struct #{@class_name} : public RegisterGroup"
        cputs indent, "{"
        cputs indent + 1, "static const char *const NAME; // #{@name}"
        cputs indent + 1, "const char * name() const {"
        cputs indent + 2, "return NAME;"
        cputs indent + 1, "}"
        cputs indent + 1, "uint32_t address() const {"
        cputs indent + 2, "return ADDRESS;"
        cputs indent + 1, "}"
        cputs indent + 1, "static const uint32_t ADDRESS; //#{"0x%x"%[@addrs[0]]}"

        @regs.each do |reg|
            reg.hxx indent + 1
        end

        cputs indent + 1, "void walk(Walker *w, Chip *c, Target *t);"
        cputs indent, "};"

        cputs indent, "#{@class_name} #{@class_instance};"
        puts ""
    end

    def hxx_array indent
        cputs indent, "struct #{@class_name} : public RegisterGroupSet"
        cputs indent, "{"
        cputs indent + 1, "static const char *const NAME; // #{@name}"
        cputs indent + 1, "const char * name() const {"
        cputs indent + 2, "return NAME;"
        cputs indent + 1, "}"
        cputs indent + 1, "uint32_t address(uint32_t idx) const {"
        cputs indent + 2, "return ADDRESSES[idx];"
        cputs indent + 1, "}"
        cputs indent + 1, "uint32_t cnt() const {"
        cputs indent + 2, "return CNT;"
        cputs indent + 1, "}"
        cputs indent + 1, "static const uint32_t ADDRESSES[#{@addrs.size}]; //{#{@addrs.map{|x| "0x%x" % [x]}.join(", ")}}"
        cputs indent + 1, "enum { CNT = #{@addrs.size} };"
        cputs indent + 1, "struct Group {"
        @regs.each do |reg|
            reg.hxx indent + 2
        end
        cputs indent + 1, "};"
        cputs indent + 1, "Group group[#{@addrs.size}];"
        cputs indent + 1, "Group& operator[](size_t s) {"
        cputs indent + 2, "return group[s];"
        cputs indent + 1, "}"
        cputs indent + 1, "void walk(Walker *w, Chip *c, Target *t);"
        cputs indent, "};"
        cputs indent, "#{@class_name} #{@class_instance};"
        puts ""
    end

    def hxx indent
        if (@addrs.size > 1)
            hxx_array indent
        else
            hxx_single indent
        end
    end

    def cxx_array ns
        puts "const uint32_t #{ns}::ADDRESSES[] = {#{@addrs.map{|x| "0x%x" % [x]}.join(", ")}};"
        @regs.each do |reg|
            reg.cxx "#{ns}::Group"
        end
    end

    def cxx_single ns
        puts "const uint32_t #{ns}::ADDRESS = #{"0x%x"%[@addrs[0]]};"
        @regs.each do |reg|
            reg.cxx ns
        end
    end

    def cxx ns
        ns = "#{ns}::#{@class_name}"
        puts "const char *const #{ns}::NAME = \"#{@name}\";"
        if (@addrs.size > 1)
            cxx_array ns
        else
            cxx_single ns
        end
        cputs 0, "void #{ns}::walk(Walker *w, Chip *c, Target *t)"
        cputs 0, "{"
        @regs.each do |reg|
            if (@addrs.size > 1)
                @addrs.size.times do |i|
                    cputs 1, "group[#{i}].#{reg.class_instance}.walk(w, c, t, this, #{i});"
                end
            else
                cputs 1, "#{reg.class_instance}.walk(w, c, t, this);"
            end
        end
        cputs 0, "}"
    end
end

class Target
    attr_accessor :name
    attr_accessor :groups
    attr_accessor :device_id
    attr_accessor :class_name
    attr_accessor :class_instance

    def initialize name, id
        if /([a-zA-Z0-9_]+)\ \(Channel\ (\d+), Device\ (0x[0-9a-fA-F]+)\)/.match(name)
            @name = $1
            @device_id = id
            @class_base_name = "#{@name.downcase}_dev#{@device_id}"
            @class_name = @class_base_name.camelize
            @class_instance = @class_base_name.camelize(:lower)
        else
            raise name
        end

        @groups = []
    end

    def parse doc, n
        # TODO, recursive groups?
        doc.find('.//reggrp').each do |reggroup|
            g = RegGrp.new reggroup['name'], reggroup['base_addr'], reggroup['repl_cnt'], reggroup['repl_width']
            g.parse reggroup
            @groups << g
        end
    end

    def print_short
        puts @name
        @groups.each do |g|
            g.print_short
        end
    end

    def reglist
        @groups.each do |g|
            g.reglist ({:target_name    => @name,
                        :target_device  => @device_id})
        end
    end

    def hxx indent
        cputs indent, "struct #{@class_name} : public Target"
        cputs indent, "{"
        cputs indent + 1, "static const char *const NAME; // #{@name}"
        cputs indent + 1, "const char * name() const {"
        cputs indent + 2, "return NAME;"
        cputs indent + 1, "}"
        cputs indent + 1, "static const uint32_t DEVICE_ID; // #{@device_id}"
        cputs indent + 1, "uint32_t device_id() const {"
        cputs indent + 2, "return DEVICE_ID;"
        cputs indent + 1, "}"
        @groups.each do |g|
            g.hxx indent + 1
        end
        cputs indent + 1, "void walk(Walker *w, Chip *c);"
        cputs indent, "};"
        cputs indent, "#{@class_name} #{@class_instance};"
        puts ""
    end

    def cxx chip
        ns = "#{chip}::#{@class_name}"
        cputs 0, "const char *const #{ns}::NAME = \"#{@name}\";"
        cputs 0, "const uint32_t #{ns}::DEVICE_ID = #{@device_id};"
        cputs 0, "void #{ns}::walk(Walker *w, Chip *c)"
        cputs 0, "{"
        @groups.each do |g|
            cputs 1, "#{g.class_instance}.walk(w, c, this);"
        end
        cputs 0, "}"
        @groups.each do |g|
            g.cxx ns
        end
    end
end

class Chip
    attr_accessor :name
    attr_accessor :targets

    def initialize cmlfile
        @targets = []
        parser = XML::Parser.file(cmlfile)
        doc = parser.parse
        @name = doc.find_first('/chip/@name').value
        @class_name = @name.camelize
        doc.find('.//target_info').each do |target|
            t = Target.new target.find_first('./target_sim_info/@name').value, target.find_first('./target_sim_info/@id').value
            t.parse(target, name)
            @targets << t
        end
    end

    def print_short
        @targets.each do |t|
            t.print_short
        end
    end

    def reglist
        @targets.each do |t|
            t.reglist
        end
    end

    def hxx
        cputs 0, "struct #{@class_name} : public Chip"
        cputs 0, "{"
        cputs 1, "static const char *const NAME; // #{@name}"
        cputs 1, "const char * name() const {"
        cputs 2, "return NAME;"
        cputs 1, "}"
        @targets.each do |t|
            t.hxx 1
        end
        cputs 1, "void walk(Walker *w);"
        cputs 0, "};"
        cputs 0, ""
    end

    def cxx
        ns = @class_name
        cputs 0, "void #{ns}::walk(Walker *w){"
        @targets.each do |t|
            cputs 1, "#{t.class_instance}.walk(w, this);"
        end
        cputs 0, "}"
        cputs 0, "const char *const #{ns}::NAME = \"#{@name}\";"
        @targets.each do |t|
            t.cxx ns
        end
    end
end

$chip = Chip.new "venice.cml"

$stdout.reopen("venice.hxx", "w")
puts "#ifndef __VENICE_HXX__"
puts "#define __VENICE_HXX__"
puts "#include \"reg_sim_base.hxx\""
$chip.hxx
puts "#endif // __VENICE_HXX__"

$stdout.reopen("venice.cxx", "w")
puts "#include \"venice.hxx\""
$chip.cxx


