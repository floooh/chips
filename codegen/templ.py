
# replace `<% templ_name %>` with templ_str
def replace(src_lines, templ_name, templ_str):
    templ_lines = templ_str.splitlines()
    res = []
    skip = False
    for src_line in src_lines:
        if skip:
            # skip src lines until template end-marker found
            if '%>' in src_line:
                skip = False
                res.append(src_line)
        else:
            res.append(src_line)
            # check for start of template
            if f'<% {templ_name}' in src_line:
                skip = True
                for templ_line in templ_lines:
                    res.append(templ_line.rstrip())
    return res
