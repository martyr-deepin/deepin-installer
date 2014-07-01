class Trie
    constructor: ->
        @words = 0
        @children = {}

    insert : (str, pos=0)->
        return if str.length == 0

        if pos == str.length
            @words++
            return

        k = str[pos]
        if not @children[k]
            @children[k] = new Trie()
        child = @children[k]
        child.insert(str, pos + 1)

    getAllWords: (str)->
        ret = []
        if @words > 0
            ret.push(str)

        i = 0
        for key, child of @children
            if child.getAllWords?
                ret = ret.concat(child.getAllWords(str + key))
        return ret

    autoComplete: (str, pos=0)->
        return [] if str.length == 0
        k = str[pos]
        child = @children[k]
        if not child
            return []
        if pos == str.length-1
            return child.getAllWords(str)
        return child.autoComplete(str,pos + 1)
