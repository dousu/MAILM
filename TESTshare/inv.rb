var = ""
File.open("editdata","r"){|io| var = io.readline}
a=var.split(",").map!{|a| a.to_i}

def rec(arr,el)
  if arr[1..-1].size == (el-1)
    if el == 1
      true
    else
      rec(arr[1..-1],arr[0])
    end
  else
    false
  end
end

puts rec(a[1..-1],a[0])
