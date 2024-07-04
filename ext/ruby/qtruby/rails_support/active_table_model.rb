=begin
This table model allows an ActiveRecord or ActiveResource to be used as a
basis for a Qt5::AbstractTableModel for viewing in a Qt5::TableView. Example
usage:

app = Qt5::Application.new(ARGV)
agencies = TravelAgency.find(:all, :conditions => [:name => 'Another Agency'])
model = ActiveTableModel.new(agencies)
table = Qt5::TableView.new
table.model = model
table.show
app.exec

Written by Richard Dale and Silvio Fonseca

=end

require 'Qt5'
require 'date'

class ActiveTableModel < Qt5::AbstractTableModel
    def initialize(collection, columns=nil)
        super()
        @collection = collection
        if columns
            if columns.kind_of? Hash
                @keys=columns.keys
                @labels=columns.values
            else
                @keys=columns
            end
        else
            @keys = build_keys([], @collection.first.attributes)
        end
        @labels||=@keys.collect { |k| k.humanize.gsub(/\./, ' ') }
    end

    def build_keys(keys, attrs, prefix="")
        attrs.inject(keys) do |cols, a|
            if a[1].respond_to? :attributes
                build_keys(cols, a[1].attributes, prefix + a[0] + ".")
            else
                cols << prefix + a[0]
            end
        end
    end

    def rowCount(parent)
        @collection.size
    end

    def columnCount(parent)
        @keys.size
    end


    def [](row)
        row = row.row if row.is_a?Qt5::ModelIndex
        @collection[row]
    end

    def column(name)
        @keys.index name
    end

    def data(index, role=Qt5::DisplayRole)
        invalid = Qt5::Variant.new
        return invalid unless role == Qt5::DisplayRole or role == Qt5::EditRole
        item = @collection[index.row]
        return invalid if item.nil?
        raise "invalid column #{index.column}" if (index.column < 0 ||
            index.column >= @keys.size)
        value = eval("item.attributes['%s']" % @keys[index.column].gsub(/\./, "'].attributes['"))
        return Qt5::Variant.new(value)
    end

    def headerData(section, orientation, role=Qt5::DisplayRole)
        invalid = Qt5::Variant.new
        return invalid unless role == Qt5::DisplayRole
        v = case orientation
        when Qt5::Horizontal
            @labels[section]
        else
            section
        end
        return Qt5::Variant.new(v)
    end

    def flags(index)
        return Qt5::ItemIsEditable | super(index)
    end

    def setData(index, variant, role=Qt5::EditRole)
        if index.valid? and role == Qt5::EditRole
            att = @keys[index.column]
            # Don't allow the primary key to be changed
            if att == 'id'
                return false
            end

            item = @collection[index.row]
            raise "invalid column #{index.column}" if (index.column < 0 ||
                index.column >= @keys.size)
            value = variant.value

            if value.class.name == "Qt5::Date"
                value = Date.new(value.year, value.month, value.day)
            elsif value.class.name == "Qt5::Time"
                value = Time.new(value.hour, value.min, value.sec)
            end

            eval("item['%s'] = value" % att.gsub(/\./, "']['"))
            item.save
            emit dataChanged(index, index)
            return true
        else
            return false
        end
    end
end

# kate: indent-width 4;
