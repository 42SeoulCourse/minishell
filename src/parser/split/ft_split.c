#include "../../../include/minishell.h"

static void	make_result_from_line(t_split *split, char *line)
{
	int		i;
	int		idx;

	i = 0;
	idx = 0;
	split->str = NULL;
	while (line[split->i] != '\0' && idx < split->split_size)
	{
		split_line(line, &str, &i);
		result[idx] = str;
		str = NULL;
		idx++;
		split->i++;
	}
}

char	**command_split(char *str)
{
	t_split		*split;
	char		**result;
	int			i;

	if (!str)
		return (NULL);
	split = safe_malloc(sizeof(t_split));
	split->result = NULL;
	split->split_size = count_split_size(str);
	split->result = safe_malloc(sizeof(char *) * (split->split_size + 1));
	split->result[split->split_size] = NULL;
	make_result_from_line(split, str);
	i = 0;
	result = safe_malloc(sizeof(char *) * (split->split_size + 1));
	result[split->split_size] = NULL;
	while (i < split->split_size)
	{
		result[i] = ft_strdup(split->result[i]);
		free(split->result[i]);
		i++;
	}
	free(split->result);
	split->result = NULL;
	free(split);
	return (result);
}
